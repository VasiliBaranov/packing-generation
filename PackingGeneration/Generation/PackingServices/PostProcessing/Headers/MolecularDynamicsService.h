// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h
#define Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h

#include "Generation/Model/Headers/Config.h"
//#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"
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
        Model::Packing originalParticles;
        const Model::ModellingContext* context;
        Model::GenerationConfig generationConfig;

        // Services
        PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep;
        PackingSerializer* packingSerializer;
        MathService* mathService;
        GeometryService* geometryService;

    public:
        MolecularDynamicsService(MathService* mathService, GeometryService* geometryService,
                PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer);

        virtual ~MolecularDynamicsService();

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        // Should be called only after config and context are set. TODO: fix here and everywhere (remove this hidden dependency).
        void SetParticles(const Model::Packing& particles);

        Model::MolecularDynamicsStatistics CalculateStationaryStatistics() const;

        Model::MolecularDynamicsStatistics CalculateImmediateStatistics() const;

        DISALLOW_COPY_AND_ASSIGN(MolecularDynamicsService);

    private:
        std::string GetEquilibratedPackingPath() const;
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_MolecularDynamicsService_h */

