// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_ParticleCollisionService_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_ParticleCollisionService_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
namespace PackingServices { struct MathService; }
namespace PackingGenerators { struct MovingParticle; }

namespace PackingGenerators
{
    // See Lubachevscky, Stillinger (1990) Geometric properties of random disk packings
    class ParticleCollisionService
    {
    private:
        PackingServices::MathService* mathService;
        Core::FLOAT_TYPE initialInnerDiameterRatio;
        Core::FLOAT_TYPE ratioGrowthRate;

    public:
        explicit ParticleCollisionService(PackingServices::MathService* mathService);

        void Initialize(Core::FLOAT_TYPE initialInnerDiameterRatio, Core::FLOAT_TYPE ratioGrowthRate);

        Core::FLOAT_TYPE GetCollisionTime(Core::FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle) const;

        // Assumes equal masses.
        // Returns exchanged momentum (if masses are unity).
        Core::FLOAT_TYPE FillVelocitiesAfterCollision(Core::FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle, Core::SpatialVector* firstVelocity, Core::SpatialVector* secondVelocity) const;

    private:
        Core::FLOAT_TYPE FillVelocitiesAfterCollisionBothMobile(Core::FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle,
                Core::SpatialVector* firstVelocity, Core::SpatialVector* secondVelocity) const;

        Core::FLOAT_TYPE FillVelocitiesAfterCollisionOneImmobile(Core::FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle,
                Core::SpatialVector* firstVelocity, Core::SpatialVector* secondVelocity) const;

        Core::FLOAT_TYPE GetCollisionTime(Core::FLOAT_TYPE currentTime, Core::FLOAT_TYPE a, Core::FLOAT_TYPE b, Core::FLOAT_TYPE c) const;

        // Returns parallel vector length
        Core::FLOAT_TYPE SplitVectorIntoComponents(const Core::SpatialVector& vector, const Core::SpatialVector& normal, Core::SpatialVector* parallelToNormal, Core::SpatialVector* transverseToNormal) const;

        void FillDifference(Core::FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle, Core::SpatialVector* difference) const;

        DISALLOW_COPY_AND_ASSIGN(ParticleCollisionService);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_ParticleCollisionService_h */

