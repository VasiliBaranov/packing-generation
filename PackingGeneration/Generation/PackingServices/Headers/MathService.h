// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_MathService_h
#define Generation_PackingServices_Headers_MathService_h

#include "Generation/Model/Headers/Types.h"
#include "Core/Headers/Macros.h"
#include "IContextDependentService.h"
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace PackingServices
{
    // Utility class for finding distance between particles.
    class MathService : public virtual IContextDependentService
    {
    private:
        const Model::SystemConfig* config;
        Core::SpatialVector packingHalfSize;

    public:
        MathService();

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        Core::FLOAT_TYPE GetNormalizedDistanceSquare(Model::ParticleIndex n1, Model::ParticleIndex n2, const Model::Packing& particles) const;

        Core::FLOAT_TYPE GetNormalizedDistanceSquare(const Model::Particle& firstParticle, const Model::Particle& secondParticle) const;

        Core::FLOAT_TYPE GetNormalizedDistance(const Model::Particle& firstParticle, const Model::Particle& secondParticle) const;

        Core::FLOAT_TYPE GetDistanceLength(const Core::SpatialVector& firstPoint, const Core::SpatialVector& secondPoint) const;

        void FillDistance(const Core::SpatialVector& to, const Core::SpatialVector& from, Core::SpatialVector* difference) const;

        void FillClosestPeriodicImagePosition(const Core::SpatialVector& stablePoint, const Core::SpatialVector& movablePoint, Core::SpatialVector* periodicImage) const;

        Core::FLOAT_TYPE GetDistanceSquare(const Core::SpatialVector& firstPoint, const Core::SpatialVector& secondPoint) const;

    private:
        void ReflectPeriodically(Core::SpatialVector* distance, int dimension) const;

        DISALLOW_COPY_AND_ASSIGN(MathService);
    };
}

#endif /* Generation_PackingServices_Headers_MathService_h */

