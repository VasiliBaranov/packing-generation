// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_BasePackingStep_h
#define Generation_PackingGenerators_Headers_BasePackingStep_h

#include "Core/Headers/Macros.h"
#include "IPackingStep.h"
namespace PackingServices { class GeometryService; }
namespace PackingServices { class MathService; }
namespace PackingServices { class IClosestPairProvider; }
namespace PackingServices { class INeighborProvider; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class BasePackingStep : public virtual IPackingStep
    {
    protected:
        // Services
        PackingServices::GeometryService* geometryService;
        PackingServices::IClosestPairProvider* closestPairProvider;
        PackingServices::MathService* mathService;

        // Context
        const Model::ModellingContext* context;
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;
        const Model::GenerationConfig* generationConfig;

        // Working variables
        Core::FLOAT_TYPE particlesVolume;
        Core::FLOAT_TYPE totalVolume;
        Core::FLOAT_TYPE theoreticalPorosity;

        Model::Packing* particles;
        Core::FLOAT_TYPE outerDiameterRatio, innerDiameterRatio;

        // Packing step parameters
        bool isOuterDiameterChanging, canOvercomeTheoreticalDensity;

    public:
        PackingServices::INeighborProvider* neighborProvider;

    public:
        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE bool ShouldContinue() const;

        OVERRIDE Core::FLOAT_TYPE GetInnerDiameterRatio() const;

        OVERRIDE Core::FLOAT_TYPE GetOuterDiameterRatio() const;

        bool IsOuterDiameterChanging() const;

        bool CanOvercomeTheoreticalDensity() const;

        ~BasePackingStep();

    protected:
        BasePackingStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::IClosestPairProvider* closestPairProvider,
                PackingServices::MathService* mathService);

        Core::FLOAT_TYPE CalculateTheoreticalPorosity() const;

        Core::FLOAT_TYPE CalculateCurrentPorosity(Core::FLOAT_TYPE diameterRatio) const;
    };
}

#endif /* Generation_PackingGenerators_Headers_BasePackingStep_h */

