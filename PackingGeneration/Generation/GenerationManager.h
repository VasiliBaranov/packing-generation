// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_GenerationManager_h
#define Generation_GenerationManager_h

#include "Core/Headers/Macros.h"
#include "Model/Headers/Types.h"
#include "Model/Headers/Config.h"

namespace PackingGenerators { class IPackingGenerator; }
namespace PackingServices { class MolecularDynamicsService; }
namespace PackingServices { class RattlerRemovalService; }
namespace PackingServices { class IEnergyService; }
namespace PackingServices { class PressureService; }
namespace PackingServices { class HessianService; }
namespace PackingServices { class InsertionRadiiGenerator; }
namespace PackingServices { struct PackingSerializer; }
namespace PackingServices { struct DistanceService; }
namespace PackingServices { struct OrderService; }
namespace PackingServices { struct ImmobileParticlesService; }

namespace Geometries { struct IGeometry; }

namespace Generation
{
    class GenerationManager;

    typedef void(GenerationManager::*Action)(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

    // Represents a high-level class (a controller) for packing generation and processing.
    class GenerationManager
    {
    private:
        PackingServices::PackingSerializer* packingSerializer;
        PackingGenerators::IPackingGenerator* packingGenerator;
        PackingServices::InsertionRadiiGenerator* insertionRadiiGenerator;
        PackingServices::DistanceService* distanceService;
        PackingServices::OrderService* orderService;
        PackingServices::IEnergyService* contractionEnergyService;
        PackingServices::HessianService* hessianService;
        PackingServices::PressureService* pressureService;
        PackingServices::MolecularDynamicsService* molecularDynamicsService;
        PackingServices::RattlerRemovalService* rattlerRemovalService;
        PackingServices::ImmobileParticlesService* immobileParticlesService;

        Core::FLOAT_TYPE innerDiameterRatio;

    public:
        GenerationManager(PackingServices::PackingSerializer* packingSerializer,
                PackingGenerators::IPackingGenerator* packingGenerator,
                PackingServices::InsertionRadiiGenerator* insertionRadiiGenerator,
                PackingServices::DistanceService* distanceService,
                PackingServices::OrderService* orderService,
                PackingServices::IEnergyService* contractionEnergyService,
                PackingServices::HessianService* hessianService,
                PackingServices::PressureService* pressureService,
                PackingServices::MolecularDynamicsService* molecularDynamicsService,
                PackingServices::RattlerRemovalService* rattlerRemovalService,
                PackingServices::ImmobileParticlesService* immobileParticlesService);

        void GeneratePacking(const Model::ExecutionConfig& userConfig);

        void GenerateInsertionRadii(const Model::ExecutionConfig& userConfig);

        void CalculateDistancesToClosestSurfaces(const Model::ExecutionConfig& userConfig);

        void CalculateContactNumberDistribution(const Model::ExecutionConfig& userConfig);

        void CalculateEntropy(const Model::ExecutionConfig& userConfig);

        void CalculateDirections(const Model::ExecutionConfig& userConfig);

        void CalculateContractionEnergies(const Model::ExecutionConfig& userConfig);

        void GenerateOrder(const Model::ExecutionConfig& userConfig);

        void CalculateHessianEigenvalues(const Model::ExecutionConfig& userConfig);

        void CalculatePressures(const Model::ExecutionConfig& userConfig);

        void CalculateMolecularDynamicsStatistics(const Model::ExecutionConfig& userConfig);

        void RemoveRattlers(const Model::ExecutionConfig& userConfig);

        void CalculatePairCorrelationFunction(const Model::ExecutionConfig& userConfig);

        void CalculateStructureFactor(const Model::ExecutionConfig& userConfig);

        void GenerateLocalOrientationalDisorder(const Model::ExecutionConfig& userConfig);

        void CalculateImmediateMolecularDynamicsStatistics(const Model::ExecutionConfig& userConfig);

        void CalculateNearestNeighbors(const Model::ExecutionConfig& userConfig);

        void CalculateActiveGeometry(const Model::ExecutionConfig& userConfig);

        void CalculateSuccessfulPermutationProbability(const Model::ExecutionConfig& userConfig);

        virtual ~GenerationManager();

    private:

        void FillFullConfig(const Model::ExecutionConfig& userConfig, Model::ExecutionConfig* fullConfig) const;

        Model::ModellingContext* CreateContext(const Model::ExecutionConfig& fullConfig) const;

        boost::shared_ptr<Geometries::IGeometry> CreateGeometry(const Model::SystemConfig& config) const;

        boost::shared_ptr<Geometries::IGeometry> CreateGeometry(const Model::SystemConfig& config, const Core::SpatialVector& shift) const;

        void FillContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;
        void AddLinearContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;
        void AddLogContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;

        void ReadOrCreatePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, bool shouldAlwaysReadPacking, Model::Packing* particles) const;

        void ReadOrCreatePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, Model::Packing* particles) const;

        void SetActiveParticlesByActiveGeometry(const Model::SystemConfig& fullConfig, const Geometries::IGeometry& geometry, Model::Packing* particles) const;

        void CreateInitialPacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, Model::Packing* particles, PackingGenerators::IPackingGenerator* initialGenerator) const;

        void ExecuteAlgorithm(const Model::ExecutionConfig& userConfig, std::string targetFileName,
                bool shouldExitIfTargetFileExists, bool shouldAlwaysReadPacking, Action algorithm);

        Core::FLOAT_TYPE GetExpectedCoordinationNumber(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles) const;

        void CreateActiveConfig(const Model::ExecutionConfig& fullConfig, Core::FLOAT_TYPE contractionFactorByParticleCenters, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) const;

        void GetActiveConfigWithParticlesCount(const Model::ExecutionConfig& fullConfig, Core::FLOAT_TYPE contractionFactorByParticleCenters, Model::Packing* particles, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) const;

        void FindActiveConfigForActiveParticlesCount(const Model::ExecutionConfig& fullConfig, int activeParticlesCount, Model::Packing* particles, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) const;

        // Actions for corresponding algorithms

        void GeneratePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void GenerateInsertionRadii(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateDistancesToClosestSurfaces(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFolderPath, Model::Packing* particles);

        void CalculateContactNumberDistribution(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateEntropy(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateDirections(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateContractionEnergies(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void GenerateOrder(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateHessianEigenvalues(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculatePressures(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateMolecularDynamicsStatistics(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void RemoveRattlers(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculatePairCorrelationFunction(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateStructureFactor(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void GenerateLocalOrientationalDisorder(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateImmediateMolecularDynamicsStatistics(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateNearestNeighbors(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateActiveGeometry(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void CalculateSuccessfulPermutationProbability(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        DISALLOW_COPY_AND_ASSIGN(GenerationManager);

    private:
        class GetActiveParticlesCountFunctor
        {
        private:
            const GenerationManager& generationManager;
            const Model::ExecutionConfig& fullConfig;
            Model::Packing* particles;
            Model::SystemConfig* activeConfig;
            Core::SpatialVector* shift;

        public:
            GetActiveParticlesCountFunctor(const GenerationManager& generationManagerParam, const Model::ExecutionConfig& fullConfigParam,
                    Model::Packing* particles, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) : generationManager(generationManagerParam), fullConfig(fullConfigParam)
            {
                this->particles = particles;
                this->activeConfig = activeConfig;
                this->shift = shift;
            };

            int operator()(Core::FLOAT_TYPE contractionFactorByParticleCenters)
            {
                generationManager.GetActiveConfigWithParticlesCount(fullConfig, contractionFactorByParticleCenters, particles, activeConfig, shift);
                return activeConfig->particlesCount;
            };
        };
    };
}

#endif /* Generation_GenerationManager_h */

