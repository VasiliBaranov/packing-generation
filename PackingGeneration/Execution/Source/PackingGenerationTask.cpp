// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingGenerationTask.h"

#include "Generation/GenerationManager.h"
#include "Core/Headers/EndiannessProvider.h"

#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/ImmobileParticlesService.h"
#include "Generation/PackingServices/DistanceServices/Headers/VerletListNeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/CellListNeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/ClosestPairProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/NaiveNeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"
#include "Generation/PackingServices/EnergyServices/Headers/EnergyService.h"
#include "Generation/PackingServices/EnergyServices/Headers/NoRattlersEnergyService.h"
#include "Generation/PackingServices/PostProcessing/Headers/RattlerRemovalService.h"
#include "Generation/PackingServices/PostProcessing/Headers/MolecularDynamicsService.h"
#include "Generation/PackingServices/PostProcessing/Headers/PressureService.h"
#include "Generation/PackingServices/PostProcessing/Headers/InsertionRadiiGenerator.h"
#include "Generation/PackingServices/PostProcessing/Headers/HessianService.h"
#include "Generation/PackingGenerators/Headers/PackingGenerator.h"

// Energy  minimization steps
#include "Generation/PackingServices/EnergyServices/Headers/HarmonicPotential.h"
#include "Generation/PackingServices/EnergyServices/Headers/ImpermeableAttractionPotential.h"
#include "Generation/PackingGenerators/Headers/DensificationStep.h"
#include "Generation/PackingGenerators/Headers/SimpleConjugateGradientStep.h"
#include "Generation/PackingGenerators/Headers/IncreasingConjugateGradientStep.h"
#include "Generation/PackingGenerators/Headers/DecreasingConjugateGradientStep.h"
#include "Generation/PackingServices/EnergyServices/Headers/BezrukovPotential.h"

// Other steps
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/LubachevsckyStillingerStep.h"
#include "Generation/PackingGenerators/Headers/BezrukovJodreyToryStep.h"
#include "Generation/PackingGenerators/Headers/ClosestJammingStep.h"
#include "Generation/PackingGenerators/Headers/OriginalJodreyToryStep.h"
#include "Generation/PackingGenerators/Headers/KhirevichJodreyToryStep.h"
#include "Generation/PackingGenerators/Headers/MonteCarloStep.h"

using namespace std;
using namespace Geometries;
using namespace PackingGenerators;
using namespace Parallelism;
using namespace Generation;
using namespace PackingServices;
using namespace Model;
using namespace Core;

namespace Execution
{
    PackingGenerationTask::PackingGenerationTask(string baseFolder, int id)
    {
        userConfig.generationConfig.baseFolder = baseFolder;
        this->id = id;
    }

    PackingGenerationTask::~PackingGenerationTask()
    {
    }

    FLOAT_TYPE PackingGenerationTask::GetWeight() const
    {
        return 1.0;
    }

    int PackingGenerationTask::GetId() const
    {
        return id;
    }

    void PackingGenerationTask::Execute()
    {
        // This function is the simplest Inversion of Control framework: manual compile-time specification and if-else.
        // You may consider using a better alternative in this function.

        EndiannessProvider endiannessProvider;
        PackingSerializer packingSerializer(&endiannessProvider);

        MathService mathService;
        GeometryCollisionService geometryCollisionService;
        GeometryService geometryService(&mathService);
        CellListNeighborProvider baseNeighborProvider(&geometryService, &geometryCollisionService);
        VerletListNeighborProvider neighborProvider(&geometryService, &mathService, &baseNeighborProvider, &geometryCollisionService);
//        NaiveNeighborProvider neighborProvider;

        ClosestPairProvider closestPairProvider(&mathService, &neighborProvider); // performance leak
        DistanceService distanceService(&mathService, &geometryService, &neighborProvider);
        EnergyService generationEnergyService(&mathService, &neighborProvider);
//        EnergyService contractionEnergyService(&mathService, &neighborProvider);
        RattlerRemovalService rattlerRemovalServiceForEnergy(&mathService, &neighborProvider);
        NoRattlersEnergyService contractionEnergyService(&mathService, &neighborProvider, &rattlerRemovalServiceForEnergy);
        LubachevsckyStillingerStep lubachevsckyStillingerStep(&geometryService, &neighborProvider, &distanceService, &mathService, &packingSerializer, &contractionEnergyService);

        // Post-processing services
        OrderService orderService(&mathService, &neighborProvider);
        PressureService pressureService(&mathService, &neighborProvider);
        InsertionRadiiGenerator insertionRadiiGenerator(&distanceService, &geometryService);
        MolecularDynamicsService molecularDynamicsService(&mathService, &geometryService, &lubachevsckyStillingerStep, &packingSerializer);
        RattlerRemovalService rattlerRemovalService(&mathService, &neighborProvider);
        HessianService hessianService(&mathService, &neighborProvider, &rattlerRemovalService);

        int minContactsCount = 3;
        contractionEnergyService.SetMinNeighborsCount(minContactsCount);
        rattlerRemovalService.SetMinNeighborsCount(minContactsCount);

        BezrukovPotential bezrukovPotential;
        HarmonicPotential harmonicPotential(2.0);
        ImpermeableAttractionPotential impermeableAttractionPotential(&harmonicPotential);

        ImmobileParticlesService immobileParticlesService(&mathService, &geometryService, &neighborProvider);

        boost::shared_ptr<IPackingStep> packingStep = CreatePackingStep(&geometryService,
                &neighborProvider,
                &distanceService,
                &mathService,
                &closestPairProvider,
                &generationEnergyService,
                &contractionEnergyService,
                &bezrukovPotential,
                &impermeableAttractionPotential,
                &harmonicPotential,
                &packingSerializer);

        PackingGenerator generator(&packingSerializer, &geometryService, &mathService, packingStep.get());

        GenerationManager generationManager(&packingSerializer,
                &generator,
                &insertionRadiiGenerator,
                &distanceService,
                &orderService,
                &contractionEnergyService,
                &hessianService,
                &pressureService,
                &molecularDynamicsService,
                &rattlerRemovalService,
                &immobileParticlesService);

        CallCorrectMethod(&generationManager);
    }

    boost::shared_ptr<IPackingStep> PackingGenerationTask::CreatePackingStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            DistanceService* distanceService,
            MathService* mathService,
            IClosestPairProvider* closestPairProvider,
            IEnergyService* generationEnergyService,
            IEnergyService* contractionEnergyService,
            IPairPotential* bezrukovPotential,
            IPairPotential* impermeableAttractionPotential,
            IPairPotential* pairPotential,
            PackingSerializer* packingSerializer) const
    {
        boost::shared_ptr<IPackingStep> packingStep;

        const GenerationConfig& generationConfig = userConfig.generationConfig;
        if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerSimple ||
                generationConfig.generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerGradualDensification ||
                generationConfig.generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerEquilibrationBetweenCompressions ||
                generationConfig.generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerConstantPower ||
                generationConfig.generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerBiazzo)
        {
            packingStep.reset(new LubachevsckyStillingerStep(geometryService, neighborProvider, distanceService, mathService, packingSerializer, contractionEnergyService));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::ForceBiasedAlgorithm)
        {
            packingStep.reset(new BezrukovJodreyToryStep(geometryService, neighborProvider, mathService, generationEnergyService, bezrukovPotential));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::OriginalJodreyTory)
        {
            packingStep.reset(new OriginalJodreyToryStep(geometryService, closestPairProvider, mathService));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::KhirevichJodreyTory)
        {
            packingStep.reset(new KhirevichJodreyToryStep(geometryService, closestPairProvider, mathService));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::MonteCarlo)
        {
            packingStep.reset(new MonteCarloStep(geometryService, distanceService, mathService));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::ClosestJammingSearch)
        {
            packingStep.reset(new ClosestJammingStep(geometryService, neighborProvider, closestPairProvider, mathService));
        }
        else if (generationConfig.generationAlgorithm == PackingGenerationAlgorithm::ConjugateGradient)
        {
#ifdef GSL_AVAILABLE
            DensificationStep packingStep(geometryService, neighborProvider, mathService, impermeableAttractionPotential, generationEnergyService); generationEnergyService.maxCloseNeighborsCount = 6;
//            IncreasingConjugateGradientStep packingStep(geometryService, neighborProvider, mathService, pairPotential, generationEnergyService);
//            SimpleConjugateGradientStep packingStep(geometryService, neighborProvider, mathService, pairPotential, generationEnergyService);
//            DecreasingConjugateGradientStep packingStep(geometryService, neighborProvider, mathService, pairPotential, generationEnergyService);
#else
            throw NotImplementedException("Gnu Scientific Library is not available on this machine or is not set up for compiler/linker options, so ConjugateGradient algorithm can not execute.");
#endif
        }

        return packingStep;
    }

    void PackingGenerationTask::CallCorrectMethod(GenerationManager* generationManager) const
    {
        //NOTE: may be use different task classes for calling different methods (derived from a BasePackingTask)? But will be too much glue code, so for now it's ok.
        const GenerationConfig& generationConfig = userConfig.generationConfig;
        if (generationConfig.executionMode == ExecutionMode::InsertionRadiiGeneration)
        {
            generationManager->GenerateInsertionRadii(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::DistancesToClosestSurfacesCalculation)
        {
            generationManager->CalculateDistancesToClosestSurfaces(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::ContactNumberDistributionCalculation)
        {
            generationManager->CalculateContactNumberDistribution(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::EntropyCalculation)
        {
            generationManager->CalculateEntropy(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::PackingGeneration)
        {
            generationManager->GeneratePacking(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::DirectionsCalculation)
        {
            generationManager->CalculateDirections(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::ContractionEnergyCalculation)
        {
            generationManager->CalculateContractionEnergies(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::OrderCalculation)
        {
            generationManager->GenerateOrder(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::HessianEigenvaluesCalculation)
        {
            generationManager->CalculateHessianEigenvalues(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::PressureCalculation)
        {
            generationManager->CalculatePressures(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::MolecularDynamicsCalculation)
        {
            generationManager->CalculateMolecularDynamicsStatistics(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::RattlerRemoval)
        {
            generationManager->RemoveRattlers(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::PairCorrelationCalculation)
        {
            generationManager->CalculatePairCorrelationFunction(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::StructureFactorCalculation)
        {
            generationManager->CalculateStructureFactor(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::LocalOrientationalDisorder)
        {
            generationManager->GenerateLocalOrientationalDisorder(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::ImmediateMolecularDynamicsCalculation)
        {
            generationManager->CalculateImmediateMolecularDynamicsStatistics(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::NearestNeighborsCalculation)
        {
            generationManager->CalculateNearestNeighbors(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::ActiveGeometryCalculation)
        {
            generationManager->CalculateActiveGeometry(userConfig);
        }
        else if (generationConfig.executionMode == ExecutionMode::SuccessfulPermutationProbability)
        {
            generationManager->CalculateSuccessfulPermutationProbability(userConfig);
        }
        else
        {
            throw NotImplementedException("Execution mode not supported.");
        }
    }
}

