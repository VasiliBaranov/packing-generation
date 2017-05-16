// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/SelfDiffusionProcessor.h"

#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/ScopedFile.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/LubachevsckyStillingerStep.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingGenerators;

namespace PackingServices
{
    SelfDiffusionProcessor::SelfDiffusionProcessor(string equilibratedPackingPath, MathService* mathService, PackingSerializer* packingSerializer,
            PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep)
    {
        this->equilibratedPackingPath = equilibratedPackingPath;

        this->mathService = mathService;
        this->packingSerializer = packingSerializer;
        this->lubachevsckyStillingerStep = lubachevsckyStillingerStep;
    }

    void SelfDiffusionProcessor::Start()
    {
        iterationIndex = -1;
        iterationIndexSinceReset = -1;
    }

    EquilibrationProcessingStatus::Type SelfDiffusionProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        /////////////////
        // Preparation
        iterationIndex++;
        iterationIndexSinceReset++;

        if (iterationIndex == 0)
        {
            originalPacking.resize(particles.size());
            referencePacking.resize(particles.size());

            Particle::CopyPackingTo(particles, &originalPacking);
        }

        if (iterationIndexSinceReset == 0)
        {
            Particle::CopyPackingTo(particles, &referencePacking);
        }

//        // TODO: move to another IEquilibrationStatisticsGatherer (CollidedPairsProcessor)
//        if (iterationsCount == 0)
//        {
//            // TODO: avoid deep copy. A dirty hack: assume that the lubachevsckyStillingerStep passed was the one that did the equilibration step
//            initialCollidedPairs = lubachevsckyStillingerStep->collidedPairs;
//        }

        /////////////////
        // Displacement calculation and serialization
        FLOAT_TYPE globalDistance = GetDistanceBetweenPackings(originalPacking, particles);
        printf("Packing displacement vs original packing: %20.15g\n", globalDistance);

        FLOAT_TYPE localDistance = GetDistanceBetweenPackings(referencePacking, particles);
        printf("Packing displacement vs reference packing: %20.15g\n", localDistance);

        // TODO: move to another IEquilibrationStatisticsGatherer (EquilibrationSerializer)
        if (!statistics.collisionErrorsExisted && (iterationIndex % packingWritePeriod == 0)) // Write not every step to improve performance. TODO: use generationConfig->stepsToWrite
        {
            packingSerializer->SerializePacking(equilibratedPackingPath, particles);
        }

        ///////////////////////////////
        // Writing equilibration history. TODO: move to another IEquilibrationStatisticsGatherer (may be EquilibrationSerializer as well)

//        const string statesFolder = "EquilibrationHistory";
//        Path::EnsureDirectory(statesFolder);
//
//        ostringstream outputStream;
//        outputStream << (iterationsCount + 1);
//        string stateFolderName = outputStream.str();
//        string stateFolderPath = Path::Append(statesFolder, stateFolderName);
//        Path::EnsureDirectory(stateFolderPath);
//
//        string differencePath = Path::Append(stateFolderPath, "difference.csv");
//        WritePackingDifference(originalPacking, currentPacking, differencePath);
//
//        string currentPackingPath = Path::Append(stateFolderPath, "packing.xyzd");
//        packingSerializer->SerializePacking(currentPackingPath, currentPacking);

        if (!resetsExternally && iterationIndexSinceReset == (diffusionCalculationPeriod - 1))
        {
            printf("Made enough iterations (%d) for self-diffusion. Enough statistics for self-diffusion...\n", diffusionCalculationPeriod);
            iterationIndexSinceReset = -1;
        }

        bool equilibrated = resetsExternally || (iterationIndex >= (diffusionCalculationPeriod - 1));

        EquilibrationProcessingStatus::Type result = equilibrated ?
                EquilibrationProcessingStatus::EnoughStatistics : EquilibrationProcessingStatus::NotEnoughStatistics;

        return result;
    }

    void SelfDiffusionProcessor::ResetReferencePacking()
    {
        iterationIndexSinceReset = -1;
        printf("Reference packing for self-diffusion reset externally\n");
    }

    bool SelfDiffusionProcessor::ResetsExternally()
    {
        return resetsExternally;
    }

    void SelfDiffusionProcessor::WriteCollidedPairs(const vector<CollidingPair>& initialCollidedPairs, const vector<CollidingPair>& collidedPairs, ParticleIndex particlesCount, int stateIndex) const
    {
        // Collided pairs distance
        vector<CollidingPair> commonPairs;
        StlUtilities::FindSetIntersection(initialCollidedPairs, collidedPairs, &commonPairs);

        FLOAT_TYPE similarity = static_cast<FLOAT_TYPE>(commonPairs.size()) / static_cast<FLOAT_TYPE>(initialCollidedPairs.size() + collidedPairs.size()) * 2.0;

        FLOAT_TYPE neighborsPerParticleByCollisions = static_cast<FLOAT_TYPE>(collidedPairs.size()) / particlesCount;
        printf("Common collided pairs ratio: %f\n", similarity);
        printf("Neighbors per particle by collisions: %f\n", neighborsPerParticleByCollisions);

//        // TODO: remove duplication
//        const string statesFolder = "EquilibrationHistory";
//        Path::EnsureDirectory(statesFolder);

//        ostringstream outputStream;
//        outputStream << stateIndex;
//        string stateFolderName = outputStream.str();
//        string stateFolderPath = Path::Append(statesFolder, stateFolderName);
//        Path::EnsureDirectory(stateFolderPath);

//        string collidingPairsPath = Path::Append(stateFolderPath, "collidedPairs.txt");
//        WriteCollidedPairs(collidedPairs, collidingPairsPath);
    }

    void SelfDiffusionProcessor::WriteCollidedPairs(const vector<CollidingPair>& collidedPairs, string filePath) const
    {
        ScopedFile<LogErrorHandler> file(filePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "particleIndex neighborIndex collisionsCount\n");

        for (vector<CollidingPair>::const_iterator it = collidedPairs.begin(); it != collidedPairs.end(); ++it)
        {
            const CollidingPair& collidingPair = *it;

            fprintf(file, "%d %d %d\n",
                    collidingPair.particleIndex,
                    collidingPair.neighborIndex,
                    collidingPair.collisionsCount);
        }
    }

    FLOAT_TYPE SelfDiffusionProcessor::GetDistanceBetweenPackings(const Packing& firstPacking, const Packing& secondPacking) const
    {
        FLOAT_TYPE distanceSquare = 0.0;
        for (size_t i = 0; i < firstPacking.size(); i++)
        {
            distanceSquare += mathService->GetDistanceSquare(firstPacking[i].coordinates, secondPacking[i].coordinates);
        }

        return sqrt(distanceSquare);
    }

    void SelfDiffusionProcessor::WritePackingDifference(const Packing& firstPacking, const Packing& secondPacking, string filePath) const
    {
        ScopedFile<LogErrorHandler> file(filePath, FileOpenMode::Write | FileOpenMode::Binary);
        if (DIMENSIONS == 3)
        {
            fprintf(file, "x, y, z, dx, dy, dz\n");

            for (size_t i = 0; i < firstPacking.size(); ++i)
            {
                SpatialVector difference;
                mathService->FillDistance(firstPacking[i].coordinates, secondPacking[i].coordinates, &difference);

                fprintf(file, "%f, %f, %f, %f, %f, %f\n",
                        firstPacking[i].coordinates[Axis::X],
                        firstPacking[i].coordinates[Axis::Y],
                        firstPacking[i].coordinates[Axis::Z],
                        difference[Axis::X],
                        difference[Axis::Y],
                        difference[Axis::Z]);
            }
        }
        if (DIMENSIONS == 2)
        {
            fprintf(file, "x, y, dx, dy\n");

            for (size_t i = 0; i < firstPacking.size(); ++i)
            {
                SpatialVector difference;
                mathService->FillDistance(firstPacking[i].coordinates, secondPacking[i].coordinates, &difference);

                fprintf(file, "%f, %f, %f, %f\n",
                        firstPacking[i].coordinates[Axis::X],
                        firstPacking[i].coordinates[Axis::Y],
                        difference[Axis::X],
                        difference[Axis::Y]);
            }
        }
    }

    void SelfDiffusionProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {
//        // TODO: move to another IEquilibrationStatisticsGatherer (CollidedPairsProcessor)
//        WriteCollidedPairs(initialCollidedPairs, Path::Append(generationConfig.baseFolder, "initialCollidedPairs.txt"));
//        WriteCollidedPairs(lubachevsckyStillingerStep->collidedPairs, Path::Append(generationConfig.baseFolder, "equilibriumCollidedPairs.txt"));
    }
}

