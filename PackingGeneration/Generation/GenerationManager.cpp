// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "GenerationManager.h"

#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "Core/Headers/Path.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Math.h"
#include "Model/Headers/Config.h"

#include "PackingServices/Headers/MathService.h"
#include "PackingServices/Headers/PackingSerializer.h"
#include "PackingServices/Headers/GeometryService.h"
#include "PackingServices/Headers/ImmobileParticlesService.h"

#include "PackingServices/PostProcessing/Headers/InsertionRadiiGenerator.h"
#include "PackingServices/PostProcessing/Headers/HessianService.h"
#include "PackingServices/PostProcessing/Headers/PressureService.h"
#include "PackingServices/PostProcessing/Headers/RattlerRemovalService.h"
#include "PackingServices/PostProcessing/Headers/MolecularDynamicsService.h"

#include "PackingServices/EnergyServices/Headers/HarmonicPotential.h"
#include "PackingServices/EnergyServices/Headers/IEnergyService.h"

#include "PackingServices/DistanceServices/Headers/DistanceService.h"

#include "Geometries/Headers/BulkGeometry.h"
#include "Geometries/Headers/CircleGeometry.h"
#include "Geometries/Headers/RectangleGeometry.h"
#include "Geometries/Headers/TrapezoidGeometry.h"

#include "PackingGenerators/InitialGenerators/Headers/BulkPoissonGenerator.h"
#include "PackingGenerators/InitialGenerators/Headers/BulkPoissonInCellsGenerator.h"
#include "PackingGenerators/InitialGenerators/Headers/HcpGenerator.h"

#include "PackingGenerators/Headers/IPackingGenerator.h"

using namespace std;
using namespace Geometries;
using namespace PackingServices;
using namespace PackingGenerators;
using namespace Model;
using namespace Core;

namespace Generation
{
    GenerationManager::GenerationManager(PackingSerializer* packingSerializer,
            IPackingGenerator* packingGenerator,
            InsertionRadiiGenerator* insertionRadiiGenerator,
            DistanceService* distanceService,
            OrderService* orderService,
            IEnergyService* contractionEnergyService,
            HessianService* hessianService,
            PressureService* pressureService,
            MolecularDynamicsService* molecularDynamicsService,
            RattlerRemovalService* rattlerRemovalService,
            ImmobileParticlesService* immobileParticlesService)
    {
        this->packingSerializer = packingSerializer;
        this->packingGenerator = packingGenerator;
        this->insertionRadiiGenerator = insertionRadiiGenerator;
        this->distanceService = distanceService;
        this->orderService = orderService;
        this->contractionEnergyService = contractionEnergyService;
        this->hessianService = hessianService;
        this->pressureService = pressureService;
        this->molecularDynamicsService = molecularDynamicsService;
        this->rattlerRemovalService = rattlerRemovalService;
        this->immobileParticlesService = immobileParticlesService;

        innerDiameterRatio = 1.0;
    }

    void GenerationManager::GeneratePacking(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, PACKING_FILE_NAME, false, false, &GenerationManager::GeneratePacking);
    }

    void GenerationManager::GenerateInsertionRadii(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, INSERTION_RADII_FILE_NAME, true, true, &GenerationManager::GenerateInsertionRadii);
    }

    void GenerationManager::CalculateDistancesToClosestSurfaces(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, DISTANCES_TO_CLOSEST_SURFACES_FOLDER_NAME, true, true, &GenerationManager::CalculateDistancesToClosestSurfaces);
    }

    void GenerationManager::CalculateContactNumberDistribution(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, CONTACT_NUMBER_DISTRIBUTION_FILE_NAME, true, true, &GenerationManager::CalculateContactNumberDistribution);
    }

    void GenerationManager::CalculateEntropy(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, ENTROPY_FILE_NAME, true, true, &GenerationManager::CalculateEntropy);
    }

    void GenerationManager::CalculateDirections(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, PARTICLE_DIRECTIONS_FILE_NAME, true, true, &GenerationManager::CalculateDirections);
    }

    void GenerationManager::CalculateContractionEnergies(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, CONTRACTION_ENERGIES_FILE_NAME, true, true, &GenerationManager::CalculateContractionEnergies);
    }

    void GenerationManager::GenerateOrder(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, ORDER_FILE_NAME, true, true, &GenerationManager::GenerateOrder);
    }

    void GenerationManager::CalculateHessianEigenvalues(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, HESSIAN_EIGENVALUES_FILE_NAME, true, true, &GenerationManager::CalculateHessianEigenvalues);
    }

    void GenerationManager::CalculatePressures(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, PRESSURES_FILE_NAME, true, true, &GenerationManager::CalculatePressures);
    }

    void GenerationManager::CalculateMolecularDynamicsStatistics(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, MOLECULAR_DYNAMICS_STATISTICS_FILE_NAME, true, true, &GenerationManager::CalculateMolecularDynamicsStatistics);
    }

    void GenerationManager::RemoveRattlers(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, "", false, true, &GenerationManager::RemoveRattlers);
    }

    void GenerationManager::CalculatePairCorrelationFunction(const Model::ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, PAIR_CORRELATION_FUNCTION_FILE_NAME, true, true, &GenerationManager::CalculatePairCorrelationFunction);
    }

    void GenerationManager::CalculateStructureFactor(const Model::ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, STRUCTURE_FACTOR_FILE_NAME, true, true, &GenerationManager::CalculateStructureFactor);
    }

    void GenerationManager::GenerateLocalOrientationalDisorder(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, LOCAL_ORIENTATIONAL_DISORDER_FILE_NAME, true, true, &GenerationManager::GenerateLocalOrientationalDisorder);
    }

    void GenerationManager::CalculateImmediateMolecularDynamicsStatistics(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, IMMEDIATE_MOLECULAR_DYNAMICS_STATISTICS_FILE_NAME, true, true, &GenerationManager::CalculateImmediateMolecularDynamicsStatistics);
    }

    void GenerationManager::CalculateNearestNeighbors(const ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, NEAREST_NEIGHBORS_FILE_NAME, true, true, &GenerationManager::CalculateNearestNeighbors);
    }

    void GenerationManager::CalculateActiveGeometry(const Model::ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, ACTIVE_GEOMETRY_FILE_NAME, true, true, &GenerationManager::CalculateActiveGeometry);
    }

    void GenerationManager::CalculateSuccessfulPermutationProbability(const Model::ExecutionConfig& userConfig)
    {
        ExecuteAlgorithm(userConfig, SUCCESSFUL_PERMUTATION_PROBABILITY_FILE_NAME, true, true, &GenerationManager::CalculateSuccessfulPermutationProbability);
    }

    void GenerationManager::ExecuteAlgorithm(const ExecutionConfig& userConfig, string targetFileName,
            bool shouldExitIfTargetFileExists, bool shouldAlwaysReadPacking, Action algorithm)
    {
        string targetFilePath = Path::Append(userConfig.generationConfig.baseFolder, targetFileName);
        if (shouldExitIfTargetFileExists && Path::Exists(targetFilePath))
        {
            return;
        }

        ExecutionConfig fullConfig;
        FillFullConfig(userConfig, &fullConfig);

        Math::SetSeed(fullConfig.generationConfig.seed);
        boost::shared_ptr<IGeometry> geometry = CreateGeometry(fullConfig.systemConfig);

        SystemConfig activeConfig;
        boost::shared_ptr<IGeometry> activeGeometry;
        string activeConfigPath = Path::Append(fullConfig.generationConfig.baseFolder, ACTIVE_GEOMETRY_FILE_NAME);
        if (Path::Exists(activeConfigPath))
        {
            SpatialVector shift;
            packingSerializer->ReadActiveConfig(activeConfigPath, &activeConfig, &shift);
            activeConfig.MergeWith(fullConfig.systemConfig);

            activeGeometry = CreateGeometry(activeConfig, shift);
        }

        ModellingContext context(&fullConfig.systemConfig, geometry.get(), activeGeometry.get());

        packingGenerator->SetContext(context);
        insertionRadiiGenerator->SetContext(context);
        distanceService->SetContext(context);
        orderService->SetContext(context);
        contractionEnergyService->SetContext(context);
        hessianService->SetContext(context);
        pressureService->SetContext(context);
        molecularDynamicsService->SetContext(context);
        rattlerRemovalService->SetContext(context);

        Packing particles;
        ReadOrCreatePacking(fullConfig, context, shouldAlwaysReadPacking, &particles);

        (this->*algorithm)(fullConfig, context, targetFilePath, &particles);
    }

    // Actions for corresponding algorithms

    void GenerationManager::GeneratePacking(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        string infoFile = Path::Append(fullConfig.generationConfig.baseFolder, PACKING_FILE_NAME_NFO);
        if (Path::Exists(infoFile))
        {
            return;
        }

//        // A quick fix for Matthias packings
//        Packing& particlesRef = *particles;
//        string nearestNeighbotsPath = Path::Append(fullConfig.generationConfig.baseFolder, NEAREST_NEIGHBORS_FILE_NAME);
//        if (Path::Exists(nearestNeighbotsPath))
//        {
//            vector<ParticlePair> closestPairs;
//            packingSerializer->ReadNearestNeighbors(nearestNeighbotsPath, &closestPairs);
//            for (size_t i = 0; i < closestPairs.size(); ++i)
//            {
//                bool radiusCorrect = closestPairs[i].normalizedDistanceSquare > 0.95;
//                particlesRef[closestPairs[i].firstParticleIndex].isImmobile = radiusCorrect;
//                particlesRef[closestPairs[i].secondParticleIndex].isImmobile = radiusCorrect;
//            }
//        }

        packingGenerator->SetGenerationConfig(fullConfig.generationConfig);
        packingGenerator->ArrangePacking(particles);

        // TODO: Think about writing info file here.
        // TODO: Think on renaming the folder, updating the generation config, rescaling the packing, updating the info file according to the innerDiameterRatio here.
        packingSerializer->SerializePacking(targetFilePath, *particles);

        innerDiameterRatio = packingGenerator->GetFinalInnerDiameterRatio();
        string contractionEnergiesFilePath = Path::Append(fullConfig.generationConfig.baseFolder, CONTRACTION_ENERGIES_FILE_NAME);
        CalculateContractionEnergies(fullConfig, context, contractionEnergiesFilePath, particles);

        innerDiameterRatio = 1.0;
    }

    void GenerationManager::GenerateInsertionRadii(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        vector<FLOAT_TYPE> insertionRadii;
        insertionRadiiGenerator->FillInsertionRadii(*particles, fullConfig.generationConfig.insertionRadiiCount, &insertionRadii);
        packingSerializer->SerializeInsertionRadii(targetFilePath, insertionRadii);
    }

    void GenerationManager::CalculateDistancesToClosestSurfaces(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFolderPath, Packing* particles)
    {
        vector<int> surfaceIndexes;
        vector<vector<FLOAT_TYPE> > distancesToSurfaces;

        int minNeighborsCount = 2; // Now: some number to ensure that we have all the numbers consecutively. Before: contractionEnergyService->GetMinNeighborsCount(); // average neighbors count in mechanically stable packings with infinite friction
        int maxNeighborsCount = 14; // max number of contacts in monodisperse packings
        int minSurfaceIndex = minNeighborsCount - 1;
        int surfaceIndexesCount = maxNeighborsCount - minNeighborsCount + 1;
        surfaceIndexes.resize(surfaceIndexesCount);
        VectorUtilities::FillLinearScale(minSurfaceIndex, &surfaceIndexes);
        surfaceIndexes.insert(surfaceIndexes.begin(), 0);

        insertionRadiiGenerator->FillDistancesToSurfaces(*particles, fullConfig.generationConfig.insertionRadiiCount, surfaceIndexes, targetFolderPath, *packingSerializer);
    }

    void GenerationManager::CalculateContactNumberDistribution(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating contact number distributions\n");
        Packing& packingToUse = *particles;

        // just make sure that the closest pair touches
        //ClosestPairProvider 
        distanceService->SetParticles(*particles);
        ParticlePair closestPair = distanceService->FindClosestPair();
        FLOAT_TYPE closestNormalizedDistance = std::sqrt(closestPair.normalizedDistanceSquare);
        printf("Normalized distance of a closest pair is %f\n", closestNormalizedDistance);
        if (closestNormalizedDistance > 1.0001)
        {
            printf("Min normalized distance is too high, parcking was probably not rescaled to the final density. Rescaling before contacts calculation...\n");
            Packing rescaledParticles = *particles;
            for (Particle& p : rescaledParticles)
            {
                p.diameter *= closestNormalizedDistance;
            }
            packingToUse = rescaledParticles;
        }

        FLOAT_TYPE contractionRate = 1.0 - 1e-4;
        vector<int> neighborCounts;
        vector<int> neighborCountFrequencies;
        vector<vector<int>> touchingParticleIndexes;
        contractionEnergyService->SetParticles(packingToUse);
        FLOAT_TYPE estimatedCoordinationNumber = insertionRadiiGenerator->GetContactNumberDistribution(packingToUse, contractionEnergyService,
                contractionRate, &neighborCounts, &neighborCountFrequencies, &touchingParticleIndexes);

        printf("Estimated coordination number is %f\n", estimatedCoordinationNumber);

        packingSerializer->SerializeContactNumberDistribution(targetFilePath, neighborCounts, neighborCountFrequencies);

        string targetFolder = Core::Path::GetParentPath(targetFilePath);
        string contactNumbersFilePath = Core::Path::Append(targetFolder, CONTACTING_NEIGHBORS_FILE_NAME);
        packingSerializer->SerializeContactingNeighborIndexes(contactNumbersFilePath, touchingParticleIndexes);

        vector<FLOAT_TYPE> normalizedContactingNeighborDistances;
        insertionRadiiGenerator->FillNormalizedContactingNeighborDistances(packingToUse, touchingParticleIndexes, &normalizedContactingNeighborDistances);

        string contactingNeighborDistancesFilePath = Core::Path::Append(targetFolder, CONTACTING_NEIGHBOR_DISTANCES_FILE_NAME);
        packingSerializer->SerializeContactingNeighborDistances(contactingNeighborDistancesFilePath, normalizedContactingNeighborDistances);

//        FLOAT_TYPE expectedCoordinationNumber = GetExpectedCoordinationNumber(fullConfig, context, targetFilePath, particles);
//        // Find the best contraction rate to get this coordination number
//        FLOAT_TYPE contractionRate = insertionRadiiGenerator->GetContractionRateForCoordinationNumber(contractionEnergyService, expectedCoordinationNumber);
//
//        // Find coordination numbers distribution
//        vector<int> neighborCounts;
//        vector<int> neighborCountFrequencies;
//        FLOAT_TYPE estimatedCoordinationNumber = insertionRadiiGenerator->GetContactNumberDistribution(*particles, contractionEnergyService, contractionRate, &neighborCounts, &neighborCountFrequencies);
//        if (std::abs(estimatedCoordinationNumber - expectedCoordinationNumber) > 1e-2)
//        {
//            printf("Coordination number estimated incorrectly\n");
//        }
//
//        packingSerializer->SerializeContactNumberDistribution(targetFilePath, neighborCounts, neighborCountFrequencies);
    }

    FLOAT_TYPE GenerationManager::GetExpectedCoordinationNumber(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles) const
    {
        string energyFilePath = Path::Append(Path::GetParentPath(targetFilePath), CONTRACTION_ENERGIES_FILE_NAME);
        if (!Path::Exists(energyFilePath))
        {
            throw InvalidOperationException("Contraction energy file does not exist.");
        }
        contractionEnergyService->SetParticles(*particles);

        // Read coordination numbers from file
        vector<FLOAT_TYPE> contractionRatios;
        vector<FLOAT_TYPE> energyPowers;
        vector<FLOAT_TYPE> contractionEnergies;
        vector<int> nonRattlersCounts;
        packingSerializer->ReadContractionEnergies(energyFilePath, &contractionRatios, &energyPowers, &contractionEnergies, &nonRattlersCounts);

        // Find expected coordination number
        FLOAT_TYPE expectedCoordinationNumber = 0;
        for (size_t i = 0; i < contractionRatios.size(); ++i)
        {
            if (energyPowers[i] > 0)
            {
                expectedCoordinationNumber = contractionEnergies[i - 1] / nonRattlersCounts[i - 1];
                break;
            }
        }

        return expectedCoordinationNumber;
    }

    void GenerationManager::CalculateEntropy(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        FLOAT_TYPE entropy = insertionRadiiGenerator->CalculateEntropy(*particles, fullConfig.generationConfig.insertionRadiiCount);
        packingSerializer->SerializeEntropy(targetFilePath, entropy);

//        vector<FLOAT_TYPE> localEntropies;
//        FLOAT_TYPE entropy = insertionRadiiGenerator->CalculateLocalEntropies(particles, fullConfig->generationConfig.insertionRadiiCount, &localEntropies);
//        packingSerializer->SerializeLocalEntropy(targetFilePath, entropy, &localEntropies);
    }

    void GenerationManager::CalculateDirections(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        vector<OrderService::NeighborDirections> particleDirections;
        orderService->SetParticles(*particles);
        orderService->FillParticleDirections(&particleDirections);
        packingSerializer->SerializeParticleDirections(targetFilePath, fullConfig.systemConfig.particlesCount, particleDirections);
    }

    void GenerationManager::CalculateContractionEnergies(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating contraction energies\n");

        vector<FLOAT_TYPE> contractionRatios;
        FillContractionRatios(&contractionRatios);
        int count = contractionRatios.size();
        vector<FLOAT_TYPE> fullContractionRatios;

        // Copy contraction rates twice
        StlUtilities::Append(contractionRatios, &fullContractionRatios);
        StlUtilities::Append(contractionRatios, &fullContractionRatios);

        const HarmonicPotential zeroPotential(0.0);
        const HarmonicPotential secondPotential(2.0);

        vector<const IPairPotential*> fullPotentials(count, &zeroPotential);
        vector<const IPairPotential*> potentials(count, &secondPotential);
        StlUtilities::Append(potentials, &fullPotentials);

        // Rescale the contraction ratios according to the final innerDiameterRatio (this method may be called after packing generation).
        vector<FLOAT_TYPE> rescaledContractionRatios;
        for (vector<FLOAT_TYPE>::iterator it = fullContractionRatios.begin(); it != fullContractionRatios.end(); ++it)
        {
            FLOAT_TYPE contractionRatio = *it;
            rescaledContractionRatios.push_back(contractionRatio / innerDiameterRatio);
        }

        contractionEnergyService->SetParticles(*particles);
        IEnergyService::EnergiesResult result = contractionEnergyService->GetContractionEnergies(rescaledContractionRatios, fullPotentials);

        // Use non-rescaled contraction ratios.
        vector<FLOAT_TYPE> fullEnergyPowers(count, 0.0);
        vector<FLOAT_TYPE> energyPowers(count, 2.0);
        StlUtilities::Append(energyPowers, &fullEnergyPowers);

        packingSerializer->SerializeContractionEnergies(targetFilePath, fullContractionRatios, fullEnergyPowers, result.contractionEnergies, result.nonRattlersCounts);
    }

    void GenerationManager::GenerateOrder(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        orderService->SetParticles(*particles);
        OrderService::Order order = orderService->GetOrder(6); // 6 is the best order, as is maximum for FCC packings
        packingSerializer->SerializeOrder(targetFilePath, order);
    }

    void GenerationManager::CalculateHessianEigenvalues(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        hessianService->SetParticles(*particles);
        vector<FLOAT_TYPE> hessianEigenvalues;
        hessianService->FillHessianEigenvalues(&hessianEigenvalues);
        packingSerializer->SerializeHessianEigenvalues(targetFilePath, hessianEigenvalues);
    }

    void GenerationManager::CalculatePressures(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        vector<FLOAT_TYPE> contractionRatios;
        FillContractionRatios(&contractionRatios);
        int count = contractionRatios.size();
        vector<FLOAT_TYPE> fullContractionRatios;

        // No sense in calculating pressure for power = 0
        StlUtilities::Append(contractionRatios, &fullContractionRatios);
        vector<FLOAT_TYPE> fullEnergyPowers(count, 2.0);

        vector<FLOAT_TYPE> pressures;
        pressureService->SetParticles(*particles);
        pressureService->FillPressures(fullContractionRatios, fullEnergyPowers, &pressures);
        packingSerializer->SerializePressures(targetFilePath, fullContractionRatios, fullEnergyPowers, pressures);
    }

    void GenerationManager::CalculateMolecularDynamicsStatistics(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating molecular dynamics statistics\n");

        molecularDynamicsService->SetGenerationConfig(fullConfig.generationConfig);
        molecularDynamicsService->SetParticles(*particles);

        MolecularDynamicsStatistics statistics = molecularDynamicsService->CalculateStationaryStatistics();
        packingSerializer->SerializeMolecularDynamicsStatistics(targetFilePath, statistics);
    }

    void GenerationManager::RemoveRattlers(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        vector<bool> rattlerMask(context.config->particlesCount);
        rattlerRemovalService->SetParticles(*particles);
//        const FLOAT_TYPE contractionRate = 1.0 - 1.0e-7;
//        const FLOAT_TYPE contractionRate = 1.0 - 1.0e-5;
        const FLOAT_TYPE contractionRate = 1.0 / 1.5; //particles that have one radius between surfaces will become in contact
        rattlerRemovalService->FillRattlerMask(contractionRate, &rattlerMask);
        int nonRattlersCount = rattlerRemovalService->FindNonRattlersCount(rattlerMask);
        Packing nonRattlerParticles;
        nonRattlerParticles.resize(nonRattlersCount);
        rattlerRemovalService->FillNonRattlerPacking(rattlerMask, &nonRattlerParticles);

        printf("Removing rattlers. Non-rattlers count: %d\n", nonRattlersCount);

        PackingInfo oldInfo;
        PackingInfo nonRattlerInfo;

        ExecutionConfig nonRattlerConfig;

        string packingInfoFilePath = Path::Append(fullConfig.generationConfig.baseFolder, PACKING_FILE_NAME_NFO);
        packingSerializer->ReadPackingInfo(packingInfoFilePath, &oldInfo);

        rattlerRemovalService->FillNonRattlerConfig(nonRattlersCount, fullConfig, &nonRattlerConfig);
        rattlerRemovalService->FillNonRattlerPackingInfo(nonRattlersCount, nonRattlerParticles, nonRattlerConfig, oldInfo, &nonRattlerInfo);

        // Serialize non-rattler packing
        packingSerializer->SerializePackingInfo(packingInfoFilePath, nonRattlerConfig.systemConfig, nonRattlerInfo);

        string packingFilePath = Path::Append(fullConfig.generationConfig.baseFolder, PACKING_FILE_NAME);
        packingSerializer->SerializePacking(packingFilePath, nonRattlerParticles);

        string configFilePath = Path::Append(fullConfig.generationConfig.baseFolder, CONFIG_FILE_NAME);
        packingSerializer->SerializeConfig(configFilePath, nonRattlerConfig);
    }

    void GenerationManager::CalculatePairCorrelationFunction(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating pair correlation function\n");

        distanceService->SetParticles(*particles);
        PairCorrelationFunction pairCorrelationFunction;
        distanceService->FillPairCorrelationFunction(&pairCorrelationFunction);

        packingSerializer->SerializePairCorrelationFunction(targetFilePath, pairCorrelationFunction);
    }

    void GenerationManager::CalculateStructureFactor(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating structure factor\n");

        Packing& particlesRef = *particles;

        std::vector<int> particleIndexesOfInterest;
        if (fullConfig.generationConfig.particlesToKeepForStructureFactor > 0)
        {
            int particlesToChooseCount = fullConfig.generationConfig.particlesToKeepForStructureFactor;
            bool keepSmallest = fullConfig.generationConfig.keepSmallParticlesForStructureFactor.value;

            // choosing that many largest particles
            // NOTE: maybe use nth element
            std::vector<double> diameters;
            for (const DomainParticle& particle : particlesRef)
            {
                diameters.push_back(particle.diameter);
            }
            std::vector<int> permutation;
            StlUtilities::SortPermutation(diameters, &permutation);

            if (keepSmallest)
            {
                permutation.erase(permutation.begin() + particlesToChooseCount, permutation.end());
            }
            else
            {
                int particlesToRemove = permutation.size() - particlesToChooseCount;
                permutation.erase(permutation.begin(), permutation.begin() + particlesToRemove);
            }

            particleIndexesOfInterest.swap(permutation);

            double minSelectedDiameter = 1000; // TODO: use numeric_limits
            double maxSelectedDiameter = 0;
            for (int i : particleIndexesOfInterest)
            {
                double diameter = particlesRef[i].diameter;
                if (diameter > maxSelectedDiameter)
                {
                    maxSelectedDiameter = diameter;
                }
                if (diameter < minSelectedDiameter)
                {
                    minSelectedDiameter = diameter;
                }
            }

            if (keepSmallest)
            {
                printf("Expected to calculate structure factor for %d smallest particles. Selected %d smallest particles. Max selected diameter: %f\n",
                    particlesToChooseCount,
                    particleIndexesOfInterest.size(),
                    maxSelectedDiameter);
            }
            else
            {
                printf("Expected to calculate structure factor for %d largest particles. Selected %d largest particles. Min selected diameter: %f\n",
                    particlesToChooseCount,
                    particleIndexesOfInterest.size(),
                    minSelectedDiameter);
            }
        }


        distanceService->SetParticles(*particles);
        StructureFactor structureFactor;
        distanceService->FillStructureFactor(particleIndexesOfInterest, &structureFactor);

        packingSerializer->SerializeStructureFactor(targetFilePath, structureFactor);
    }

    void GenerationManager::GenerateLocalOrientationalDisorder(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating local orientational disorder\n");

        orderService->SetParticles(*particles);
        OrderService::LocalOrientationalDisorder localOrientationalDisorder;
        orderService->FillLocalOrientationalDisorder(&localOrientationalDisorder);
        packingSerializer->SerializeLocalOrientationalDisorder(targetFilePath, localOrientationalDisorder);
        packingSerializer->SerializeCloseNeighbors(Path::Append(fullConfig.generationConfig.baseFolder, "close_neighbors.txt"), localOrientationalDisorder);
    }

    void GenerationManager::CalculateImmediateMolecularDynamicsStatistics(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating immediate molecular dynamics statistics\n");

        molecularDynamicsService->SetGenerationConfig(fullConfig.generationConfig);
        molecularDynamicsService->SetParticles(*particles);

        MolecularDynamicsStatistics statistics = molecularDynamicsService->CalculateImmediateStatistics();
        packingSerializer->SerializeMolecularDynamicsStatistics(targetFilePath, statistics);
    }

    void GenerationManager::CalculateNearestNeighbors(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles)
    {
        printf("Calculating nearest neighbors\n");

        distanceService->SetParticles(*particles);
        vector<ParticlePair> closestPairs;
        distanceService->FillClosestPairs(&closestPairs);

        vector<bool> isImmobileMask(fullConfig.systemConfig.particlesCount);
        vector<int> isImmobileIntMask(fullConfig.systemConfig.particlesCount);

        VectorUtilities::InitializeWith(&isImmobileMask, false);
        VectorUtilities::InitializeWith(&isImmobileIntMask, 0);

        Model::Packing& particlesRef = *particles;
        for (ParticleIndex i = 0; i < fullConfig.systemConfig.particlesCount; ++i)
        {
            isImmobileMask[i] = particlesRef[i].isImmobile;
            isImmobileIntMask[i] = particlesRef[i].isImmobile ? 1 : 0;
        }

        vector<int> permutation;
        StlUtilities::SortPermutation(isImmobileIntMask, &permutation);

        vector<ParticlePair> sortedClosestPairs;
        StlUtilities::Permute(closestPairs, permutation, &sortedClosestPairs);

        vector<bool> sortedIsImmobileMask;
        StlUtilities::Permute(isImmobileMask, permutation, &sortedIsImmobileMask);

        packingSerializer->SerializeNearestNeighbors(targetFilePath, closestPairs, sortedIsImmobileMask);
    }

    void GenerationManager::CreateActiveConfig(const ExecutionConfig& fullConfig, FLOAT_TYPE contractionFactorByParticleCenters, SystemConfig* activeConfig, SpatialVector* shift) const
    {
        SystemConfig& activeConfigRef = *activeConfig;
        activeConfigRef.Reset();
        activeConfigRef.MergeWith(fullConfig.systemConfig);

        FLOAT_TYPE meanParticleRadius = 0.5; // TODO: determine better
        // contractionFactor is measured by the true radius of the bounding cylinder (i.e., by particle surfaces)
        FLOAT_TYPE contractionFactor = ((activeConfigRef.packingSize[Axis::X] - 2.0 * meanParticleRadius) * contractionFactorByParticleCenters + 2.0 * meanParticleRadius) / activeConfigRef.packingSize[Axis::X];
        SpatialVector contractionFactors;
        VectorUtilities::InitializeWith(&contractionFactors, contractionFactor);

//            contractionFactors[DIMENSIONS - 1] = 1.0; // For packings that are periodic by z. TODO: periodicity shall be specified in generation.conf. IGeometry shall include this information (as done in LatticeGeoetry)
        contractionFactors[DIMENSIONS - 1] = (activeConfigRef.packingSize[DIMENSIONS - 1] - 8.0 * meanParticleRadius) / activeConfigRef.packingSize[DIMENSIONS - 1]; // Remove 2 diameters from each side
        VectorUtilities::Multiply(activeConfigRef.packingSize, contractionFactors, &activeConfigRef.packingSize);

        SpatialVector margins;
        VectorUtilities::Subtract(fullConfig.systemConfig.packingSize, activeConfigRef.packingSize, &margins);
        VectorUtilities::MultiplyByValue(margins, 0.5, shift);
    }

    void GenerationManager::GetActiveConfigWithParticlesCount(const ExecutionConfig& fullConfig, FLOAT_TYPE contractionFactorByParticleCenters, Packing* particles, SystemConfig* activeConfig, SpatialVector* shift) const
    {
        SystemConfig& activeConfigRef = *activeConfig;
        // the default active area is 0.5 of the cylinder, if cylinder radius is measured by particle centers
        CreateActiveConfig(fullConfig, contractionFactorByParticleCenters, activeConfig, shift);
        boost::shared_ptr<IGeometry> activeGeometry = CreateGeometry(activeConfigRef, *shift);

        ////////////////////
        // Set active geometry

        // NOTE: here I use a dirty hack and do not call SetContext for all the services, as I know, that non of the services caches the active geometry.
        // TODO: extract setting context from ExecuteAlgorithm to a separate method, call here.

        // Set correct isImmobile mask
        SetActiveParticlesByActiveGeometry(fullConfig.systemConfig, *activeGeometry.get(), particles);

        ////////////////////
        // Determine the amount of active non-rattler particles

        // Find the best contraction rate to get the expected coordination number
//        FLOAT_TYPE expectedCoordinationNumber = GetExpectedCoordinationNumber(fullConfig, context, targetFilePath, particles);
//        FLOAT_TYPE contractionRate = insertionRadiiGenerator->GetContractionRateForCoordinationNumber(contractionEnergyService, expectedCoordinationNumber);
        FLOAT_TYPE contractionRate = 1.0 - 1e-7;

        vector<bool> rattlerMask;
        rattlerRemovalService->SetParticles(*particles);
        rattlerRemovalService->FillRattlerMask(contractionRate, &rattlerMask);

        int activeNonRattlersCount = 0;
        Packing& particlesRef = *particles;
        for (ParticleIndex i = 0; i < fullConfig.systemConfig.particlesCount; ++i)
        {
//            if (!particlesRef[i].isImmobile && !rattlerMask[i])
            if (!particlesRef[i].isImmobile)
            {
                activeNonRattlersCount++;
            }
        }

        activeConfigRef.particlesCount = activeNonRattlersCount;
    }

    void GenerationManager::FindActiveConfigForActiveParticlesCount(const ExecutionConfig& fullConfig, int activeParticlesCount, Packing* particles, SystemConfig* activeConfig, SpatialVector* shift) const
    {
        GetActiveParticlesCountFunctor getActiveParticlesCountFunctor(*this, fullConfig, particles, activeConfig, shift);
        StlUtilities::DoBinarySearch(0.0, 1.0, activeParticlesCount, 0, 1e-7, getActiveParticlesCountFunctor);
    }

    void GenerationManager::CalculateActiveGeometry(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        printf("Calculating active geometry\n");

        SpatialVector shift;
        SystemConfig activeConfig;

        // NOTE: this is a dirty hack. I'm using insertionRadiiCount parameter to store expectedActiveParticlesCount
        // TODO: add a separate config parameter
        int expectedActiveParticlesCount = fullConfig.generationConfig.insertionRadiiCount;

        // The usual workflow is the following:
        // 1. run CalculateActiveGeometry without an expectedActiveParticlesCount to determine approximate amount of particles inside the active geometry.
        // 2. select the min amount of activeParticlesCount
        // 3. rename the old active configs
        // 4. run CalculateActiveGeometry with expectedActiveParticlesCount = this min number
        // It ensures that all the active geometries have equal amount of active particles.
        // It will make the entropy and compactivity calculations easier
        // (actually, i'm not sure how to calculate compactivity and how to account for rattlers if the amount of particles varies).
        // TODO: extract this entire procedure to a separate method (it's hard currently,
        // as requires processing of several packings at once, which is completely unsupported by the architecture)
        if (expectedActiveParticlesCount == -1)
        {
            // the default active area is 0.5 of the cylinder, if cylinder radius is measured by particle centers
            FLOAT_TYPE contractionFactorByParticleCenters = 0.5;
            GetActiveConfigWithParticlesCount(fullConfig, contractionFactorByParticleCenters, particles, &activeConfig, &shift);
        }
        else
        {
            FindActiveConfigForActiveParticlesCount(fullConfig, expectedActiveParticlesCount, particles, &activeConfig, &shift);
        }

        packingSerializer->SerializeActiveConfig(targetFilePath, activeConfig, shift);
    }

    void GenerationManager::CalculateSuccessfulPermutationProbability(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles)
    {
        printf("Calculating successful permutation probability\n");
        Packing& packingToUse = *particles;

        int maxPermutations = 5000;
        FLOAT_TYPE successfulPermutationProbability = insertionRadiiGenerator->GetSuccessfulPermutationProbability(particles, maxPermutations);

        printf("Estimated successful permutation probability is %f\n", successfulPermutationProbability);

        //packingSerializer->SerializeContactNumberDistribution(targetFilePath, neighborCounts, neighborCountFrequencies);
    }

    void GenerationManager::FillContractionRatios(vector<FLOAT_TYPE>* contractionRatios) const
    {
        contractionRatios->clear();
        AddLogContractionRatios(contractionRatios);
//        AddLinearContractionRatios(contractionRatios);
        contractionRatios->push_back(0.9999999); // predefined value

        StlUtilities::SortAndResizeToUnique(contractionRatios);
    }

    void GenerationManager::AddLogContractionRatios(vector<FLOAT_TYPE>* contractionRatios) const
    {
        const int pointsCount = 15;
        const FLOAT_TYPE minContractionRatio = 0.9999999;
        const FLOAT_TYPE maxContractionRatio = 0.99995;

        vector<FLOAT_TYPE> contractionRatiosArray(pointsCount);

        VectorUtilities::FillLogScale(1. - minContractionRatio, 1. - maxContractionRatio, &contractionRatiosArray);
        VectorUtilities::MultiplyByValue(contractionRatiosArray, -1.0, &contractionRatiosArray);
        VectorUtilities::AddValue(contractionRatiosArray, 1.0, &contractionRatiosArray);

        StlUtilities::Append(contractionRatiosArray, contractionRatios);
    }

    void GenerationManager::AddLinearContractionRatios(vector<FLOAT_TYPE>* contractionRatios) const
    {
        // 200 points per 0.01
        // maxParticleDiameter / maxContractionRatio * innerDiameterRatio <= maxParticleDiameter * VerletListNeighborProvider::MAX_EXPECTED_OUTER_DIAMETER_RATIO + cutoffDistance
        // Approximately 1.0 / maxContractionRatio * innerDiameterRatio <= VerletListNeighborProvider::MAX_EXPECTED_OUTER_DIAMETER_RATIO
        const int pointsCount = 401;
        const FLOAT_TYPE minContractionRatio = 1.0;
        const FLOAT_TYPE maxContractionRatio = 0.98;

        vector<FLOAT_TYPE> contractionRatiosArray(pointsCount);

        VectorUtilities::FillLinearScale(minContractionRatio, maxContractionRatio, &contractionRatiosArray);
        StlUtilities::Append(contractionRatiosArray, contractionRatios);
    }

    void GenerationManager::FillFullConfig(const ExecutionConfig& userConfig, ExecutionConfig* fullConfig) const
    {
        ExecutionConfig fileConfig;
        packingSerializer->ReadConfig(userConfig.generationConfig.baseFolder, &fileConfig);

        fullConfig->MergeWith(userConfig);
        fullConfig->MergeWith(fileConfig);
    }

    boost::shared_ptr<IGeometry> GenerationManager::CreateGeometry(const SystemConfig& config, const Core::SpatialVector& shift) const
    {
        // TODO: use shift in all non-bulk geometries and may be add a template parameter to all of them <TSupportsShift>.
        boost::shared_ptr<IGeometry> geometry;
        switch (config.boundariesMode)
        {
        case BoundariesMode::Bulk:
            geometry.reset(new BulkGeometry(config));
            break;
        case BoundariesMode::Ellipse:
            geometry.reset(new CircleGeometry(config, shift));
            break;
        case BoundariesMode::Rectangle:
            geometry.reset(new RectangleGeometry(config));
            break;
        case BoundariesMode::Trapezoid:
            geometry.reset(new TrapezoidGeometry(config));
            break;
        case BoundariesMode::Unknown:
            break;
        }

        return geometry;
    }

    boost::shared_ptr<IGeometry> GenerationManager::CreateGeometry(const SystemConfig& config) const
    {
        SpatialVector shift;
        VectorUtilities::InitializeWith(&shift, 0.0);
        return CreateGeometry(config, shift);
    }

    void GenerationManager::ReadOrCreatePacking(const ExecutionConfig& fullConfig, const ModellingContext& context, bool shouldAlwaysReadPacking, Packing* particles) const
    {
        Packing& particlesRef = *particles;
        particlesRef.resize(fullConfig.systemConfig.particlesCount);

        for (ParticleIndex i = 0; i < fullConfig.systemConfig.particlesCount; ++i)
        {
            particlesRef[i].index = i;
        }

        if (shouldAlwaysReadPacking)
        {
            // Always read the packing, even if the generation mode is "start"
            packingSerializer->ReadPacking(Path::Append(fullConfig.generationConfig.baseFolder, PACKING_FILE_NAME), particles);
        }
        else
        {
            ReadOrCreatePacking(fullConfig, context, particles);
        }

        // Setting immobile particles in different ways

        Nullable<bool> shouldSuppressCrystallization = fullConfig.generationConfig.shouldSuppressCrystallization;
        if (shouldSuppressCrystallization.hasValue && shouldSuppressCrystallization.value)
        {
            immobileParticlesService->SetContext(context);
            immobileParticlesService->SetAndArrangeImmobileParticles(particles);
        }

        string immobileParticlesPath = Path::Append(fullConfig.generationConfig.baseFolder, IMMOBILE_PARTICLES_FILE_NAME);
        if (Path::Exists(immobileParticlesPath))
        {
            vector<ParticleIndex> immobileParticleIndexes;
            packingSerializer->ReadImmobileParticleIndexes(immobileParticlesPath, &immobileParticleIndexes);
            for (std::size_t immobileParticleIndexIndex = 0; immobileParticleIndexIndex < immobileParticleIndexes.size(); ++immobileParticleIndexIndex)
            {
                ParticleIndex immobileParticleIndex = immobileParticleIndexes[immobileParticleIndexIndex];
                particlesRef[immobileParticleIndex].isImmobile = true;
            }
        }

        if (context.activeGeometry != NULL)
        {
            SetActiveParticlesByActiveGeometry(fullConfig.systemConfig, *context.activeGeometry, particles);
        }
    }

    void GenerationManager::SetActiveParticlesByActiveGeometry(const SystemConfig& systemConfig, const IGeometry& activeGeometry, Packing* particles) const
    {
        Packing& particlesRef = *particles;

        int immobileParticlesCount = 0;
        for (ParticleIndex i = 0; i < systemConfig.particlesCount; ++i)
        {
            particlesRef[i].isImmobile = !activeGeometry.IsSphereInside(particlesRef[i].coordinates, particlesRef[i].diameter * 0.5);
            if (particlesRef[i].isImmobile)
            {
                immobileParticlesCount++;
            }
        }

        if (immobileParticlesCount > 0)
        {
            printf("mobileParticlesCount = %d, immobileParticlesCount = %d\n", systemConfig.particlesCount - immobileParticlesCount, immobileParticlesCount);
        }
    }

    void GenerationManager::ReadOrCreatePacking(const ExecutionConfig& fullConfig, const ModellingContext& context, Packing* particles) const
    {
        string baseFolder = fullConfig.generationConfig.baseFolder;
        int particlesCount = fullConfig.systemConfig.particlesCount;
        particles->resize(particlesCount);

        if (fullConfig.generationConfig.shouldStartGeneration.hasValue && fullConfig.generationConfig.shouldStartGeneration.value)
        {
            packingSerializer->ReadParticleDiameters(Path::Append(baseFolder, DIAMETERS_FILE_NAME), particles);

            if (fullConfig.generationConfig.initialParticleDistribution == InitialParticleDistribution::Poisson)
            {
                // TODO: create PoissonGenerator PoissonInCellsGenerator for all geometries, just call IsSphereInside for the given geometry until the sphere is in the geometry.
                BulkPoissonGenerator initialGenerator;
                CreateInitialPacking(fullConfig, context, particles, &initialGenerator);
            }
            else
            {
                BulkPoissonInCellsGenerator initialGenerator;
                CreateInitialPacking(fullConfig, context, particles, &initialGenerator);
            }

//            HcpGenerator initialGenerator;
//            CreateInitialPacking(fullConfig, context, particles, &initialGenerator);

            packingSerializer->SerializePacking(Path::Append(baseFolder, INIT_PACKING_FILE_NAME), *particles);
        }
        else
        {
            packingSerializer->ReadPacking(Path::Append(baseFolder, PACKING_FILE_NAME), particles);
        }
    }

    void GenerationManager::CreateInitialPacking(const ExecutionConfig& fullConfig, const ModellingContext& context, Packing* particles, IPackingGenerator* initialGenerator) const
    {
        initialGenerator->SetContext(context);
        initialGenerator->SetGenerationConfig(fullConfig.generationConfig);
        initialGenerator->ArrangePacking(particles);
    }

    GenerationManager::~GenerationManager()
    {

    }
}

