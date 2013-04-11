// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_GeometryService_h
#define Generation_PackingServices_Headers_GeometryService_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "IContextDependentService.h"
namespace PackingServices { class MathService; }
namespace Model { class SystemConfig; }

namespace PackingServices
{
    // Represents a class which contains utility methods for packing geometry manipulation. TODO: think of a better name.
    class GeometryService : public virtual IContextDependentService
    {
    private:
        const Model::SystemConfig* config;
        MathService* mathService;

    public:
        explicit GeometryService(MathService* mathService);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        Core::FLOAT_TYPE GetParticlesVolume(const Model::Packing& particles) const;

        void ScaleDiameters(Model::Packing* particles, Core::FLOAT_TYPE scalingFactor) const;

        Core::FLOAT_TYPE GetMaxParticleDiameter(const Model::Packing& particles) const;

        Core::FLOAT_TYPE GetMinParticleDiameter(const Model::Packing& particles) const;

        Core::FLOAT_TYPE GetMeanParticleDiameter(const Model::Packing& particles) const;

        Core::FLOAT_TYPE GetScalingFactor(Core::FLOAT_TYPE currentDensity, Core::FLOAT_TYPE targetDensity) const;

        Core::FLOAT_TYPE GetScalingFactorByPorosity(Core::FLOAT_TYPE currentPorosity, Core::FLOAT_TYPE targetPorosity) const;

        virtual ~GeometryService();

        // These methods do not require config. TODO: remove config from them
        Core::FLOAT_TYPE GetParticlesVolume(const Model::Packing& particles, const Model::SystemConfig& config) const;

        Core::FLOAT_TYPE GetPorosity(const Model::Packing& particles, const Model::SystemConfig& config) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(GeometryService);
    };
}

#endif /* Generation_PackingServices_Headers_GeometryService_h */

