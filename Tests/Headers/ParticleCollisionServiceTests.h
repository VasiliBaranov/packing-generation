// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_ParticleCollisionServiceTests_h
#define Headers_ParticleCollisionServiceTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"
namespace PackingServices { class MathService; }
namespace PackingGenerators { class ParticleCollisionService; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace Tests
{
    class ParticleCollisionServiceTests
    {
    private:
        static boost::shared_ptr<PackingServices::MathService> mathService;
        static boost::shared_ptr<PackingGenerators::ParticleCollisionService> particleCollisionService;
        static boost::shared_ptr<Geometries::IGeometry> geometry;
        static boost::shared_ptr<Model::SystemConfig> config;
        static boost::shared_ptr<Model::ModellingContext> context;

        static PackingGenerators::MovingParticle first;
        static PackingGenerators::MovingParticle second;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void GetCollisionTime_FirstParticleMovingAlongXSecondStill_TimeCorrect();

        static void GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowLinear_TimeCorrect();

        static void GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowQuadratic_TimeCorrect();

        static void FillVelocitiesAfterCollision_FirstParticleMovingAlongXSecondStill_VelocitiesSwitch();

        static void FillVelocitiesAfterCollision_ParticlesStillButGrowing_VelocitiesSpreadThem();
    };
}


#endif /* Headers_ParticleCollisionServiceTests_h */
