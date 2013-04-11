// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h
#define Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h

#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace PackingServices { struct PackingSerializer; }
namespace PackingGenerators { class LubachevsckyStillingerStep; }
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryService; }

namespace PackingServices
{
    class MolecularDynamicsService : public virtual IContextDependentService
    {
    private:
        mutable Model::Packing particles;
        mutable std::vector<PackingGenerators::CollidingPair> initialCollidedPairs;
        Model::Packing originalParticles;
        const Model::ModellingContext* context;
        Model::GenerationConfig generationConfig;

        // Services
        PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep;
        PackingSerializer* packingSerializer;
        MathService* mathService;
        GeometryService* geometryService;

    public:
        MolecularDynamicsService(MathService* mathService, GeometryService* geometryService, PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer);

        virtual ~MolecularDynamicsService();

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        // Should be called only after config and context are set. TODO: fix here and everywhere (remove this hidden dependency).
        void SetParticles(const Model::Packing& particles);

        Model::MolecularDynamicsStatistics CalculateStationaryStatistics() const;

        Model::MolecularDynamicsStatistics CalculateStatisticsWithLocking() const;

        void FillInitialPressuresAfterStrain(std::vector<Model::PressureData>* initialPressures) const;

    private:
        Core::FLOAT_TYPE GetDistanceBetweenPackings(const Model::Packing& firstPacking, const Model::Packing& secondPacking) const;

        std::string GetEquilibratedPackingPath() const;

        void WritePackingDifference(const Model::Packing& firstPacking, const Model::Packing& secondPacking, std::string filePath) const;

        void WriteCollidedPairs(const std::vector<PackingGenerators::CollidingPair>& collidedPairs, std::string filePath) const;

        void WriteCurrentState(const Model::Packing& originalPacking, const Model::Packing& currentPacking, int stateIndex) const;

        void WriteCollidedPairs(const std::vector<PackingGenerators::CollidingPair>& initialCollidedPairs, const std::vector<PackingGenerators::CollidingPair>& collidedPairs, Model::ParticleIndex particlesCount, int stateIndex) const;

        void FillContractionRatios(std::vector<Core::FLOAT_TYPE>* densities,
                std::vector<Core::FLOAT_TYPE>* contractionRatios,
                std::vector<Core::FLOAT_TYPE>* relativeContractionRatios) const;

        DISALLOW_COPY_AND_ASSIGN(MolecularDynamicsService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h */

