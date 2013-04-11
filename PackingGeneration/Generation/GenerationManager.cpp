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
            RattlerRemovalService* rattlerRemovalService)
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
        ModellingContext context(&fullConfig.systemConfig, geometry.get());

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
//        MolecularDynamicsStatistics statistics = molecularDynamicsService->CalculateStatisticsWithLocking();
        packingSerializer->SerializeMolecularDynamicsStatistics(targetFilePath, statistics);

//        vector<PressureData> pressuresData;
//        molecularDynamicsService->FillInitialPressuresAfterStrain(&pressuresData);
//        packingSerializer->SerializeMolecularDynamicsStatistics(targetFilePath, &pressuresData);
    }

    void GenerationManager::RemoveRattlers(const ExecutionConfig& fullConfig, const ModellingContext& context, string targetFilePath, Packing* particles)
    {
        vector<bool> rattlerMask(context.config->particlesCount);
        rattlerRemovalService->SetParticles(*particles);
        const FLOAT_TYPE contractionRate = 1.0 - 1.0e-7;
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

        distanceService->SetParticles(*particles);
        StructureFactor structureFactor;
        distanceService->FillStructureFactor(&structureFactor);

        packingSerializer->SerializeStructureFactor(targetFilePath, structureFactor);
    }

    void GenerationManager::FillContractionRatios(vector<FLOAT_TYPE>* contractionRatios) const
    {
        contractionRatios->clear();
        AddLogContractionRatios(contractionRatios);
        AddLinearContractionRatios(contractionRatios);
        contractionRatios->push_back(0.999); // predefined value

        StlUtilities::ResizeToUnique(contractionRatios);
        StlUtilities::Sort(contractionRatios);
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

    boost::shared_ptr<IGeometry> GenerationManager::CreateGeometry(const SystemConfig& config) const
    {
        printf("\n\n\n");

        bool unknownBoundariesMode = (config.boundariesMode != BoundariesMode::Bulk) &&
                (config.boundariesMode != BoundariesMode::Ellipse) &&
                (config.boundariesMode != BoundariesMode::Rectangle) &&
                (config.boundariesMode != BoundariesMode::Trapezoid);

        if (unknownBoundariesMode)
        {
            printf("Right now boundaries modes 1 (periodic XYZ), 2 (periodic Z, circle XY (Y is diameter) ), 3 (periodic Z, rectangle XY), or 4 (periodic Z, trapezoid XY) are supported  only\n");
            throw InvalidOperationException("Incorrect boundary mode.");
        }

        boost::shared_ptr<IGeometry> geometry;
        switch (config.boundariesMode)
        {
        case BoundariesMode::Bulk:
            geometry.reset(new BulkGeometry(config));
            break;
        case BoundariesMode::Ellipse:
            geometry.reset(new CircleGeometry(config));
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
    }

    void GenerationManager::ReadOrCreatePacking(const ExecutionConfig& fullConfig, const ModellingContext& context, Packing* particles) const
    {
        string baseFolder = fullConfig.generationConfig.baseFolder;
        int particlesCount = fullConfig.systemConfig.particlesCount;
        particles->resize(particlesCount);

        if (fullConfig.generationConfig.generationStart)
        {
            packingSerializer->ReadParticleDiameters(Path::Append(baseFolder, DIAMETERS_FILE_NAME), particles);

            if (fullConfig.generationConfig.initialParticleDistribution == InitialParticleDistribution::Poisson)
            {
                BulkPoissonGenerator initialGenerator;
                CreateInitialPacking(fullConfig, context, particles, &initialGenerator);
            }
            else
            {
                BulkPoissonInCellsGenerator initialGenerator;
                CreateInitialPacking(fullConfig, context, particles, &initialGenerator);
            }

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

