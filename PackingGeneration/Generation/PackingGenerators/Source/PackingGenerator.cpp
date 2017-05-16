// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingGenerator.h"

#include <cmath>
#include "Core/Headers/Path.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/MpiManager.h"
#include "../Headers/IPackingStep.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/Geometries/Headers/IGeometry.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/DistanceServices/Headers/VerletListNeighborProvider.h"

using namespace std;
using namespace Geometries;
using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace Generation;

namespace PackingGenerators
{
    const FLOAT_TYPE PackingGenerator::EPSILON = 1e-6;

    PackingGenerator::PackingGenerator(PackingSerializer* packingSerializer,
            GeometryService* geometryService,
            MathService* mathService,
            IPackingStep* packingStep)
    {
        this->packingSerializer = packingSerializer;
        this->geometryService = geometryService;
        this->mathService = mathService;
        this->packingStep = packingStep;
    }

    PackingGenerator::~PackingGenerator()
    {

    }

    void PackingGenerator::SetGenerationConfig(const GenerationConfig& generationConfig)
    {
        this->generationConfig = &generationConfig;
        packingStep->SetGenerationConfig(generationConfig);
    }

    void PackingGenerator::SetContext(const ModellingContext& context)
    {
        this->context = &context;
        this->config = context.config;
        this->geometry = context.geometry;

        geometryService->SetContext(context);
        mathService->SetContext(context);
        packingStep->SetContext(context);
    }

    // TODO: Refactor!
    void PackingGenerator::ArrangePacking(Packing* particles)
    {
        this->config = config;
        this->geometry = geometry;
        this->particles = particles;

        unsigned long long iterationIndex = 0L;
        clock_t startTime = clock();
        clock_t delay = 0.0;
        bool shouldContinue = true;

        Initialize();

        if (!packingStep->ShouldContinue())
        {
            printf("Packing is correct, generation not started.\n");
            CheckIntersectionsNaive();
            Finish(0, 0);
            return;
        }

        while (shouldContinue)
        {
            packingStep->DisplaceParticles();
            shouldContinue = packingStep->ShouldContinue();
            innerDiameterRatio = packingStep->GetInnerDiameterRatio();
            outerDiameterRatio = packingStep->GetOuterDiameterRatio();

            // TODO: make this check specific for each neighbor provider. Or even better: allow initial max outerDiameterRatio specification for neighbor providers.
            // Even better: allow dynamic max outerDiameterRatio specification for neighbor providers.
            if (outerDiameterRatio > VerletListNeighborProvider::MAX_EXPECTED_OUTER_DIAMETER_RATIO)
            {
                printf("WARNING: The outerDiameterRatio exceeds the one expected by VerletListNeighborProvider.\n");
                // throw InvalidOperationException("The outerDiameterRatio exceeds the one expected with Verlet List.");
            }

            if ((iterationIndex % generationConfig->stepsToWrite) == 0)
            {
                clock_t currentDelay = Log(iterationIndex);
                delay = delay + currentDelay;
            }

            iterationIndex++;
        }

        DisplayPorosity();
        CheckIntersectionsNaive();

        clock_t totalTime = clock() - startTime - delay;
        Finish(totalTime, iterationIndex);
    }

    FLOAT_TYPE PackingGenerator::GetFinalInnerDiameterRatio() const
    {
        return innerDiameterRatio;
    }

    void PackingGenerator::CheckIntersectionsNaive() const
    {
        printf("Checking min particle distance in a naive way...\n");
        ParticlePair closestPair = geometryService->GetMinNormalizedDistanceNaive(*particles);
        FLOAT_TYPE minNormalizedDistance = sqrt(closestPair.normalizedDistanceSquare);

        if (std::abs(minNormalizedDistance - innerDiameterRatio) > EPSILON)
        {
            printf("ERROR: innerDiameterRatio %g is not equal to min normalized distance from naive computation %g in the pair %d %d. Probably bugs in distance provider.\n",
                    innerDiameterRatio, minNormalizedDistance, closestPair.firstParticleIndex, closestPair.secondParticleIndex);
            throw InvalidOperationException("InnerDiameterRatio is not equal to min normalized distance from naive computation.");
        }
    }

    void PackingGenerator::DisplayPorosity() const
    {
        FLOAT_TYPE currentPorosity = CalculateCurrentPorosity(innerDiameterRatio);

        printf("Checking...\n");
        printf("Calc. porosity is %g\n", currentPorosity);
        printf("Theoretical porosity is %g\n", theoreticalPorosity);
    }

    void PackingGenerator::Finish(clock_t totalTime, unsigned long long iterationCounter) const
    {
        FLOAT_TYPE currentPorosity = CalculateCurrentPorosity(innerDiameterRatio);
        string infoFilePath = Path::Append(generationConfig->baseFolder, PACKING_FILE_NAME_NFO);
        PackingInfo info;
        info.theoreticalPorosity = theoreticalPorosity;
        info.calculatedPorosity = currentPorosity;
        info.tolerance = TOLERANCE;
        info.totalTime = totalTime / static_cast<double>(CLOCKS_PER_SEC);
        info.iterationsCount = iterationCounter;
        packingSerializer->SerializePackingInfo(infoFilePath, *config, info);

        printf("Finish:\n");
        printf("Calc. porosity is %16.15g\n", currentPorosity);
        printf("Theoretical porosity is %g\n", theoreticalPorosity);
        printf("Inner diameter ratio is %17.15g\n", innerDiameterRatio);
        printf("Time: %g s, iterations are %llu\n", info.totalTime, iterationCounter);
    }

    clock_t PackingGenerator::Log(unsigned long long iterationCounter) const
    {
        clock_t startTime = clock();
        printf("Step %llu. Inner diameter ratio is %1.15f. Outer diameter ratio is %1.15f. Writing int. packing state...", iterationCounter, innerDiameterRatio, outerDiameterRatio);
        printf("Contraction rate: %e\n", generationConfig->contractionRate);

        string packingFilePath = Path::Append(generationConfig->baseFolder, PACKING_FILE_NAME);
        // Renaming the previously saved packing. Though all IO is set up to flush the buffer immediately,
        // packing files may be empty, when the program is terminated (at least by the MPI timeout).
        // May be I should switch to MPI IO?
        if (Path::Exists(packingFilePath))
        {
            string previousPackingFilePath = Path::Append(generationConfig->baseFolder, PREVIOUS_PACKING_FILE_NAME);
            if (Path::Exists(packingFilePath))
            {
                Path::DeleteFile(previousPackingFilePath);
            }
            Path::Rename(packingFilePath, previousPackingFilePath);
        }
        packingSerializer->SerializePacking(packingFilePath, *particles);

        printf("done.\n");
        return clock() - startTime;
    }

    FLOAT_TYPE PackingGenerator::CalculateTheoreticalPorosity() const
    {
        return CalculateCurrentPorosity(1.0);
    }

    FLOAT_TYPE PackingGenerator::CalculateCurrentPorosity(FLOAT_TYPE diameterRatio) const
    {
        FLOAT_TYPE calculatedPorosity = 1.0 - (particlesVolume * std::pow(diameterRatio, DIMENSIONS)) / totalVolume;
        return calculatedPorosity;
    }

    void PackingGenerator::Initialize()
    {
        // Init variables
        particlesVolume = geometryService->GetParticlesVolume(*particles);
        totalVolume = geometry->GetTotalVolume();
        theoreticalPorosity = CalculateTheoreticalPorosity();

        // Set packing step
        packingStep->SetParticles(particles);
        innerDiameterRatio = packingStep->GetInnerDiameterRatio();
        outerDiameterRatio = packingStep->GetOuterDiameterRatio();

        // Display initial data
        printf("Contraction rate is %g\n", generationConfig->contractionRate);
        printf("Total volume of particles is %g\n", particlesVolume);
        printf("Theoretical porosity is %g\n", theoreticalPorosity);
        printf("Global minimum is %g\n", innerDiameterRatio);
    }
}

