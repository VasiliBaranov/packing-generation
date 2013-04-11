// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MolecularDynamicsService.h"

#include <numeric>

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/ScopedFile.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/LubachevsckyStillingerStep.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/DistanceServices/Headers/VerletListNeighborProvider.h"
#include "Generation/PackingServices/Headers/GeometryService.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingGenerators;

namespace PackingServices
{
    MolecularDynamicsService::MolecularDynamicsService(MathService* mathService, GeometryService* geometryService, LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer)
    {
        this->lubachevsckyStillingerStep = lubachevsckyStillingerStep;
        this->packingSerializer = packingSerializer;
        this->mathService = mathService;
        this->geometryService = geometryService;

//        lubachevsckyStillingerStep->lockParticles = true; // to disallow too big departure of a packing configuration from the initial one
//        lubachevsckyStillingerStep->hasInfiniteConductivity = true;
//        lubachevsckyStillingerStep->scaleTemperature = true;

        // Otherwise we will equilibrate packings different from the generated ones, and with a different density.
        // For generation it's OK to use as large diameter as possible.
        lubachevsckyStillingerStep->preserveInitialDiameter = true;

        // Can't put 20 * 100 as eventsPerParticle, as then LSStep doesn't predict collisions correctly (probably because total step time becomes ~1-5s,
        // while time between events can be as low as 1e-15, so machine precision errors occur).
        lubachevsckyStillingerStep->eventsPerParticle = 20;
    }

    MolecularDynamicsService::~MolecularDynamicsService()
    {
    }

    void MolecularDynamicsService::SetContext(const Model::ModellingContext& context)
    {
        this->context = &context;
        lubachevsckyStillingerStep->SetContext(context);
    }

    void MolecularDynamicsService::SetGenerationConfig(const Model::GenerationConfig& generationConfig)
    {
        this->generationConfig.Reset();
        this->generationConfig.MergeWith(generationConfig);
        this->generationConfig.contractionRate = 0.0;

        lubachevsckyStillingerStep->SetGenerationConfig(this->generationConfig);
    }

    void MolecularDynamicsService::SetParticles(const Packing& particles)
    {
        this->particles.resize(context->config->particlesCount);
        Particle::CopyPackingTo(particles, &this->particles);

        string equilibratedPackingPath = GetEquilibratedPackingPath();
        bool equilibratedPackingExists = Path::Exists(equilibratedPackingPath);
        if (equilibratedPackingExists)
        {
            printf("Equilibrated packing exists\n");
            packingSerializer->ReadPacking(equilibratedPackingPath, &this->particles);
        }

        originalParticles.resize(context->config->particlesCount);
        Particle::CopyPackingTo(this->particles, &originalParticles);

        lubachevsckyStillingerStep->SetParticles(&this->particles);
    }

    void MolecularDynamicsService::FillInitialPressuresAfterStrain(vector<PressureData>* initialPressures) const
    {
        vector<FLOAT_TYPE> densities;
        vector<FLOAT_TYPE> contractionRatios;
        vector<FLOAT_TYPE> relativeContractionRatios;

        FillContractionRatios(&densities, &contractionRatios, &relativeContractionRatios);
        int densitiesCount = densities.size();

        vector<PressureData>& initialPressuresRef = *initialPressures;
        initialPressuresRef.resize(densitiesCount);

        for (int i = 0; i < densitiesCount; i++)
        {
            Particle::CopyPackingTo(originalParticles, &particles);
            geometryService->ScaleDiameters(&particles, 1.0 / contractionRatios[i]);
            lubachevsckyStillingerStep->SetParticles(&particles);

            lubachevsckyStillingerStep->DisplaceParticles();

            PressureData pressureData;
            pressureData.contractionRatio = contractionRatios[i];
            pressureData.density = densities[i];
            pressureData.pressure = lubachevsckyStillingerStep->statistics.reducedPressure;
            pressureData.collisionErrorsExisted = lubachevsckyStillingerStep->statistics.collisionErrorsExisted;
            pressureData.jammingDensity = pressureData.density * pow(1.0 + 1.0 / (pressureData.pressure - 1.0), 3.0);

            printf("%20.15g %20.15g %20.15g %20.15g\n",
                                pressureData.contractionRatio, pressureData.density, pressureData.pressure, pressureData.jammingDensity);

            initialPressuresRef[i] = pressureData;
        }
    }

    void MolecularDynamicsService::FillContractionRatios(vector<FLOAT_TYPE>* densities,
                    vector<FLOAT_TYPE>* contractionRatios,
                    vector<FLOAT_TYPE>* relativeContractionRatios) const
    {
        const int densitiesCount = 50;
        const FLOAT_TYPE densityRangeWidth = 0.1; // will produce 0.6 for 0.64

        vector<FLOAT_TYPE>& densitiesRef = *densities;
        vector<FLOAT_TYPE>& contractionRatiosRef = *contractionRatios;
        vector<FLOAT_TYPE>& relativeContractionRatiosRef = *relativeContractionRatios;

        densitiesRef.resize(densitiesCount, 0.0);
        contractionRatiosRef.resize(densitiesCount, 0.0);
        relativeContractionRatiosRef.resize(densitiesCount, 0.0);

        FLOAT_TYPE originalDensity = 1.0 - geometryService->GetPorosity(particles, *context->config);

        VectorUtilities::FillLinearScale(originalDensity - densityRangeWidth, originalDensity, densities);

        for (int i = 0; i < densitiesCount; i++)
        {
            contractionRatiosRef[i] = geometryService->GetScalingFactor(originalDensity, densitiesRef[i]);
        }

        relativeContractionRatiosRef[0] = contractionRatiosRef[0];
        for (int i = 1; i < densitiesCount; i++)
        {
            relativeContractionRatiosRef[i] = contractionRatiosRef[i] / contractionRatiosRef[i - 1];
//            relativeContractionRatiosRef[i] = geometryService->GetScalingFactor(densitiesRef[i - 1], densitiesRef[i]);
        }
    }

    Model::MolecularDynamicsStatistics MolecularDynamicsService::CalculateStatisticsWithLocking() const
    {
        VerletListNeighborProvider* neighborProvider = dynamic_cast<VerletListNeighborProvider*>(lubachevsckyStillingerStep->neighborProvider);
        if (neighborProvider == NULL)
        {
            throw InvalidOperationException("CalculateStatisticsWithLocking supports just VerletListNeighborProvider.");
        }

        FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particles);
        FLOAT_TYPE cutoffDistance = 0.0;

        MolecularDynamicsStatistics previousStatistics;
        MolecularDynamicsStatistics currentStatistics;
        while (true)
        {
            cutoffDistance += 0.01 * meanDiameter;
            neighborProvider->SetCutoffDistance(cutoffDistance);
            printf("Cutoff distance updated and is %f\n", cutoffDistance);
            lubachevsckyStillingerStep->SetParticles(&particles); // to call its neighborProvider->SetParticles again

            previousStatistics = currentStatistics;
            currentStatistics = CalculateStationaryStatistics();

            if (cutoffDistance >= meanDiameter)
            {
                break;
            }

            if (cutoffDistance >= 0.2 * meanDiameter)
            {
                FLOAT_TYPE relativeChange = std::abs(currentStatistics.reducedPressure - previousStatistics.reducedPressure) / currentStatistics.reducedPressure;
                if (relativeChange < 1e-4)
                {
                    break;
                }
            }
        }

        return currentStatistics;
    }

    // Calculates stationary pressure statistics. TODO: refactor
    MolecularDynamicsStatistics MolecularDynamicsService::CalculateStationaryStatistics() const
    {
        const int averagingWindowWidth = 100;
//        const int averagingWindowWidth = 1;
        bool errorFrequencyHigh = false;
        int equilibrationEventsCount = 0;
        vector<FLOAT_TYPE> pressures;
        bool initialCollidingPairsSet = false;

        while (true)
        {
            int errorCyclesCount = 0;
            for (int i = 0; i < averagingWindowWidth; ++i)
            {
                lubachevsckyStillingerStep->DisplaceParticles();
                MolecularDynamicsStatistics statistics = lubachevsckyStillingerStep->statistics;
                equilibrationEventsCount += statistics.eventsCount;

                if (!initialCollidingPairsSet)
                {
                    // TODO: avoid deep copy!
                    initialCollidedPairs = lubachevsckyStillingerStep->collidedPairs;
                    initialCollidingPairsSet = true;
                }

                pressures.push_back(statistics.reducedPressure);

                if (statistics.collisionErrorsExisted)
                {
                    errorCyclesCount++;
                }

                WriteCurrentState(originalParticles, particles, pressures.size());
//                vector<CollidingPair>& collidedPairs = lubachevsckyStillingerStep->collidedPairs;
//                WriteCollidedPairs(&initialCollidedPairs, &collidedPairs, context->config->particlesCount, pressures.size());
            }

            const FLOAT_TYPE maxErrorFrequency = 0.5;
            FLOAT_TYPE errorFrequency = static_cast<FLOAT_TYPE>(errorCyclesCount) / averagingWindowWidth;
            errorFrequencyHigh= errorFrequency > maxErrorFrequency;
            if (errorFrequencyHigh)
            {
                printf("WARNING: frequency of cycles with errors is %f and is > %f. Terminating...\n", errorFrequency, maxErrorFrequency);
                break;
            }

            bool derivativeAvailable = (pressures.size() >= 2.0 * averagingWindowWidth);
            if (derivativeAvailable)
            {
                FLOAT_TYPE previousPressure = accumulate(pressures.end() - 2 * averagingWindowWidth, pressures.end() - averagingWindowWidth, 0.0) / averagingWindowWidth;
                FLOAT_TYPE currentPressure = accumulate(pressures.end() - averagingWindowWidth, pressures.end(), 0.0) / averagingWindowWidth;
                FLOAT_TYPE relativeChange = std::abs(currentPressure - previousPressure) / ( (previousPressure + currentPressure) * 0.5 );
                printf("Relative change: %f\n", relativeChange);
                const FLOAT_TYPE maxRelativeChange = 1e-4;
//                const FLOAT_TYPE maxRelativeChange = 2.0;
//                const FLOAT_TYPE maxRelativeChange = 0.05;
                if (relativeChange < maxRelativeChange)
                {
                    printf("Relative change is < %g. Terminating...\n", maxRelativeChange);
                    break;
                }
            }
        };

        MolecularDynamicsStatistics statistics = lubachevsckyStillingerStep->statistics;
        statistics.equilibrationEventsCount = equilibrationEventsCount;
        statistics.reducedPressure = accumulate(pressures.end() - averagingWindowWidth, pressures.end(), 0.0) / averagingWindowWidth;
        statistics.collisionErrorsExisted = errorFrequencyHigh;
        if (!statistics.collisionErrorsExisted)
        {
            packingSerializer->SerializePacking(GetEquilibratedPackingPath(), particles);
        }

        WriteCollidedPairs(initialCollidedPairs, Path::Append(generationConfig.baseFolder, "initialCollidedPairs.txt"));
        WriteCollidedPairs(lubachevsckyStillingerStep->collidedPairs, Path::Append(generationConfig.baseFolder, "equilibriumCollidedPairs.txt"));

        return statistics;
    }

    void MolecularDynamicsService::WriteCurrentState(const Packing& originalPacking, const Packing& currentPacking, int stateIndex) const
    {
        FLOAT_TYPE distance = GetDistanceBetweenPackings(originalPacking, currentPacking);
        printf("Packing displacement: %20.15g\n", distance);
//
//        const string statesFolder = "EquilibrationHistory";
//        Path::EnsureDirectory(statesFolder);
//
//        ostringstream outputStream;
//        outputStream << stateIndex;
//        string stateFolderName = outputStream.str();
//        string stateFolderPath = Path::Append(statesFolder, stateFolderName);
//        Path::EnsureDirectory(stateFolderPath);
//
//        string differencePath = Path::Append(stateFolderPath, "difference.csv");
//        WritePackingDifference(originalPacking, currentPacking, differencePath);
//
//        string currentPackingPath = Path::Append(stateFolderPath, "packing.xyzd");
//        packingSerializer->SerializePacking(currentPackingPath, currentPacking);
    }

    void MolecularDynamicsService::WriteCollidedPairs(const vector<CollidingPair>& initialCollidedPairs, const vector<CollidingPair>& collidedPairs, ParticleIndex particlesCount, int stateIndex) const
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

    void MolecularDynamicsService::WriteCollidedPairs(const vector<CollidingPair>& collidedPairs, string filePath) const
    {
        ScopedFile<LogErrorHandler> file(filePath, "wb");
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

    FLOAT_TYPE MolecularDynamicsService::GetDistanceBetweenPackings(const Packing& firstPacking, const Packing& secondPacking) const
    {
        FLOAT_TYPE distanceSquare = 0.0;
        for (size_t i = 0; i < firstPacking.size(); i++)
        {
            distanceSquare += mathService->GetDistanceSquare(firstPacking[i].coordinates, secondPacking[i].coordinates);
        }

        return sqrt(distanceSquare);
    }

    void MolecularDynamicsService::WritePackingDifference(const Packing& firstPacking, const Packing& secondPacking, string filePath) const
    {
        ScopedFile<LogErrorHandler> file(filePath, "wb");
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

    string MolecularDynamicsService::GetEquilibratedPackingPath() const
    {
        string equilibratedPackingPath = Path::Append(generationConfig.baseFolder, "packing_equilibrated.xyzd");
        return equilibratedPackingPath;
    }
}

