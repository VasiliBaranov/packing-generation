// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_GenerationManager_h
#define Generation_GenerationManager_h

#include "Core/Headers/Macros.h"
#include "Model/Headers/Types.h"

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
namespace Model { struct ExecutionConfig; }
namespace Model { struct ModellingContext; }
namespace Model { struct SystemConfig; }
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
                PackingServices::RattlerRemovalService* rattlerRemovalService);

        void GeneratePacking(const Model::ExecutionConfig& userConfig);

        void GenerateInsertionRadii(const Model::ExecutionConfig& userConfig);

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

        virtual ~GenerationManager();

    private:

        void FillFullConfig(const Model::ExecutionConfig& userConfig, Model::ExecutionConfig* fullConfig) const;

        Model::ModellingContext* CreateContext(const Model::ExecutionConfig& fullConfig) const;

        boost::shared_ptr<Geometries::IGeometry> CreateGeometry(const Model::SystemConfig& config) const;

        void FillContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;
        void AddLinearContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;
        void AddLogContractionRatios(std::vector<Core::FLOAT_TYPE>* contractionRatios) const;

        void ReadOrCreatePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, bool shouldAlwaysReadPacking, Model::Packing* particles) const;

        void ReadOrCreatePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, Model::Packing* particles) const;

        void CreateInitialPacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, Model::Packing* particles, PackingGenerators::IPackingGenerator* initialGenerator) const;

        void ExecuteAlgorithm(const Model::ExecutionConfig& userConfig, std::string targetFileName,
                bool shouldExitIfTargetFileExists, bool shouldAlwaysReadPacking, Action algorithm);

        // Actions for corresponding algorithms

        void GeneratePacking(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

        void GenerateInsertionRadii(const Model::ExecutionConfig& fullConfig, const Model::ModellingContext& context, std::string targetFilePath, Model::Packing* particles);

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

        DISALLOW_COPY_AND_ASSIGN(GenerationManager);
    };
}

#endif /* Generation_GenerationManager_h */

