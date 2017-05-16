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
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"

#include "Generation/PackingServices/PostProcessing/Headers/EquilibrationPressureProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/IntermediateScatteringFunctionProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/SelfDiffusionProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/ErrorRateProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/MinIterationsProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/ScatterAndDiffusionProcessor.h"

//#include <ctime>

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingGenerators;

namespace PackingServices
{
    MolecularDynamicsService::MolecularDynamicsService(MathService* mathService, GeometryService* geometryService,
            LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer)
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

        // Debug. Also useful for alpha-relaxation times calculation, because for small densities they decrease very rapidly.
        // E.g., for Pareto packing with std=0.05 and density = 0.437, 10 iterations are usually enough to decorrelate (equilibrate) the packing (t_alpha < 0.15).
//        lubachevsckyStillingerStep->eventsPerParticle = 1;
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

        VerletListNeighborProvider* neighborProvider = dynamic_cast<VerletListNeighborProvider*>(lubachevsckyStillingerStep->neighborProvider);
        if (neighborProvider != NULL)
        {
            FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particles);
            FLOAT_TYPE cutoffDistance = 2.0 * meanDiameter;
            neighborProvider->SetCutoffDistance(cutoffDistance);
            printf("Cutoff distance updated and is %f\n", cutoffDistance);
        }

        lubachevsckyStillingerStep->SetParticles(&this->particles);
    }

    MolecularDynamicsStatistics MolecularDynamicsService::CalculateImmediateStatistics() const
    {
        lubachevsckyStillingerStep->DisplaceParticles();
        MolecularDynamicsStatistics statistics = lubachevsckyStillingerStep->statistics;
        statistics.equilibrationEventsCount = statistics.eventsCount;
        return statistics;
    }

    // Calculates stationary pressure statistics. TODO: refactor
    MolecularDynamicsStatistics MolecularDynamicsService::CalculateStationaryStatistics() const
    {
        int minEquilibrationCyclesCount = generationConfig.insertionRadiiCount; // TODO: introduce a separate field in the config!!! By default it is -1.

        EquilibrationPressureProcessor pressureProcessor(lubachevsckyStillingerStep->eventsPerParticle);
        IntermediateScatteringFunctionProcessor intermediateScatteringFunctionProcessor(mathService, geometryService, lubachevsckyStillingerStep, packingSerializer, *context, generationConfig);
        SelfDiffusionProcessor selfDiffusionProcessor(GetEquilibratedPackingPath(), mathService, packingSerializer, lubachevsckyStillingerStep);
        ErrorRateProcessor errorRateProcessor;
        MinIterationsProcessor minIterationsProcessor(minEquilibrationCyclesCount);
        ScatterAndDiffusionProcessor scatterAndDiffusionProcessor(&intermediateScatteringFunctionProcessor, &selfDiffusionProcessor);

        boost::array<IEquilibrationStatisticsGatherer*, 4> statisticsGatherers = {{&pressureProcessor, &scatterAndDiffusionProcessor, &errorRateProcessor, &minIterationsProcessor}};

        for (size_t i = 0; i < statisticsGatherers.size(); ++i)
        {
            IEquilibrationStatisticsGatherer* gatherer = statisticsGatherers[i];
            gatherer->Start(); // TODO: pass the initial packing
        }

        MolecularDynamicsStatistics statistics;
        statistics.collisionErrorsExisted = false;
        unsigned long long equilibrationEventsCount = 0;

        bool errorsFound = false;
        bool notEnoughStatisticsFound = false;
        while (true)
        {
//            std::clock_t start = std::clock();

            lubachevsckyStillingerStep->DisplaceParticles();

//            std::clock_t end = std::clock();
//            double duration = (end - start) / (double)CLOCKS_PER_SEC;
//            printf ("LS takes %d clicks (%f seconds)\n", end - start, duration);

            statistics = lubachevsckyStillingerStep->statistics;
            equilibrationEventsCount += statistics.eventsCount; // TODO: move to TotalEventsCountGatherer

//            start = std::clock();

            errorsFound = false;
            notEnoughStatisticsFound = false;
            for (size_t i = 0; i < statisticsGatherers.size(); ++i)
            {
                IEquilibrationStatisticsGatherer* gatherer = statisticsGatherers[i];
                EquilibrationProcessingStatus::Type processingStatus = gatherer->ProcessStep(this->particles, statistics);

                errorsFound = errorsFound || (processingStatus == EquilibrationProcessingStatus::ErrorsFound);
                notEnoughStatisticsFound = notEnoughStatisticsFound || (processingStatus == EquilibrationProcessingStatus::NotEnoughStatistics);
            }

//            end = std::clock();
//            duration = (end - start) / (double)CLOCKS_PER_SEC;
//            printf ("Analysis takes %d clicks (%f seconds)\n", end - start, duration);

            if (errorsFound || !notEnoughStatisticsFound)
            {
                break;
            }
        }

        // I write intermediate packing in the SelfDiffusionProcessor. TODO: unify.
        // It's logical to write here, because i use the joint error flag from all the processors.
        // And SelfDiffusionProcessor uses only the pure LS error.
        // But SelfDiffusionProcessor saves also packing differences, which belong to it.
        // Shall i introduce a separate class of processing steps that use the joint error and then add a Serializer as such a step?
        if (!errorsFound)
        // if (!statistics.collisionErrorsExisted && !errorsFound) // this version is not consistent with setting statistics.collisionErrorsExisted = errorsFound below
        {
            packingSerializer->SerializePacking(GetEquilibratedPackingPath(), particles);
        }

        statistics.equilibrationEventsCount = equilibrationEventsCount;
        statistics.collisionErrorsExisted = errorsFound;

        for (size_t i = 0; i < statisticsGatherers.size(); ++i)
        {
            IEquilibrationStatisticsGatherer* gatherer = statisticsGatherers[i];
            gatherer->Finish(&statistics);
        }

        return statistics;
    }

    string MolecularDynamicsService::GetEquilibratedPackingPath() const
    {
        string equilibratedPackingPath = Path::Append(generationConfig.baseFolder, "packing_equilibrated.xyzd");
        return equilibratedPackingPath;
    }
}

