// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ParticleCollisionServiceTests.h"

#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/ParticleCollisionService.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/Model/Headers/Config.h"
#include "../Headers/Assert.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;
using namespace PackingGenerators;

namespace Tests
{
    boost::shared_ptr<MathService> ParticleCollisionServiceTests::mathService;
    boost::shared_ptr<ParticleCollisionService> ParticleCollisionServiceTests::particleCollisionService;
    boost::shared_ptr<IGeometry> ParticleCollisionServiceTests::geometry;
    boost::shared_ptr<SystemConfig> ParticleCollisionServiceTests::config;
    boost::shared_ptr<ModellingContext> ParticleCollisionServiceTests::context;

    MovingParticle ParticleCollisionServiceTests::first;
    MovingParticle ParticleCollisionServiceTests::second;

    void ParticleCollisionServiceTests::SetUp()
    {
        mathService.reset(new MathService());
        particleCollisionService.reset(new ParticleCollisionService(mathService.get()));
        particleCollisionService->Initialize(1.0, 0.0);

        config.reset(new SystemConfig());
        SpatialVector packignSize = REMOVE_LAST_DIMENSION_IF_NEEDED(10, 10, 10);
        config->packingSize = packignSize;
        config->particlesCount = 2;
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config.get()));
        context.reset(new ModellingContext(config.get(), geometry.get()));

        mathService->SetContext(*context.get());

        SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0.5, 0.5);
        SpatialVector v1 = REMOVE_LAST_DIMENSION_IF_NEEDED(1, 0, 0);
        first.coordinates = c1;
        first.velocity = v1;
        first.diameter = 1;
        first.lastEventTime = 0;

        SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(2, 0.5, 0.5);
        SpatialVector v2 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);
        second.coordinates = c2;
        second.velocity = v2;
        second.diameter = 1;
        second.lastEventTime = 0;
    }

    void ParticleCollisionServiceTests::TearDown()
    {
    }

    void ParticleCollisionServiceTests::GetCollisionTime_FirstParticleMovingAlongXSecondStill_TimeCorrect()
    {
        SetUp();

        FLOAT_TYPE time = particleCollisionService->GetCollisionTime(0, first, second);
        FLOAT_TYPE expectedTime = (second.coordinates[Axis::X] - first.coordinates[Axis::X] - 0.5 * first.diameter - 0.5 * second.diameter) / first.velocity[Axis::X];

        Assert::AreAlmostEqual(time, expectedTime, "GetCollisionTime_FirstParticleMovingAlongXSecondStill_TimeCorrect");
        TearDown();
    }

    void ParticleCollisionServiceTests::GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowLinear_TimeCorrect()
    {
        SetUp();
        FLOAT_TYPE ratioGrowthRate = 1.0;
        particleCollisionService->Initialize(1.0, ratioGrowthRate);
        FLOAT_TYPE particleRadiiGrowthRate = 0.5 * ratioGrowthRate * (first.diameter + second.diameter);

        // Collision equation is linear, not quadratic, for these parameters.
        FLOAT_TYPE time = particleCollisionService->GetCollisionTime(0, first, second);
        FLOAT_TYPE expectedTime = (second.coordinates[Axis::X] - first.coordinates[Axis::X] - 0.5 * first.diameter - 0.5 * second.diameter) / (first.velocity[Axis::X] + particleRadiiGrowthRate);

        Assert::AreAlmostEqual(time, expectedTime, "GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowLinear_TimeCorrect");
        TearDown();
    }

    void ParticleCollisionServiceTests::GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowQuadratic_TimeCorrect()
    {
        SetUp();
        FLOAT_TYPE ratioGrowthRate = 0.5;
        particleCollisionService->Initialize(1.0, ratioGrowthRate);
        FLOAT_TYPE particleRadiiGrowthRate = 0.5 * ratioGrowthRate * (first.diameter + second.diameter);

        // Collision equation is quadratic these parameters.
        FLOAT_TYPE time = particleCollisionService->GetCollisionTime(0, first, second);
        FLOAT_TYPE expectedTime = (second.coordinates[Axis::X] - first.coordinates[Axis::X] - 0.5 * first.diameter - 0.5 * second.diameter) / (first.velocity[Axis::X] + particleRadiiGrowthRate);

        Assert::AreAlmostEqual(time, expectedTime, "GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowQuadratic_TimeCorrect");
        TearDown();
    }

    void ParticleCollisionServiceTests::FillVelocitiesAfterCollision_ParticlesStillButGrowing_VelocitiesSpreadThem()
    {
        SetUp();
        first.velocity[Axis::X] = 0;
        FLOAT_TYPE ratioGrowthRate = 1.0;
        particleCollisionService->Initialize(1.0, ratioGrowthRate);

        SpatialVector firstVelocity;
        SpatialVector secondVelocity;
        particleCollisionService->FillVelocitiesAfterCollision(0, first, second, &firstVelocity, &secondVelocity);

        const string name = "FillVelocitiesAfterCollision_ParticlesStillButGrowing_VelocitiesSpreadThem";
        Assert::IsLessThanZero(firstVelocity[0], name);
        Assert::AreAlmostEqual(0.0, firstVelocity[1], name);

        Assert::IsGreaterThanZero(secondVelocity[0], name);
        Assert::AreAlmostEqual(0.0, secondVelocity[1], name);

        if (DIMENSIONS == 3)
        {
            Assert::AreAlmostEqual(0.0, firstVelocity[2], name);
            Assert::AreAlmostEqual(0.0, secondVelocity[2], name);
        }

        TearDown();
    }

    void ParticleCollisionServiceTests::FillVelocitiesAfterCollision_FirstParticleMovingAlongXSecondStill_VelocitiesSwitch()
    {
        SetUp();

        SpatialVector firstVelocity;
        SpatialVector secondVelocity;

        particleCollisionService->FillVelocitiesAfterCollision(0, first, second, &firstVelocity, &secondVelocity);

        // Velocities should be flipped
        SpatialVector firstDifference;
        SpatialVector secondDifference;

        VectorUtilities::Subtract(firstVelocity, second.velocity, &firstDifference);
        VectorUtilities::Subtract(secondVelocity, first.velocity, &secondDifference);

        FLOAT_TYPE firstDifferenceLength = VectorUtilities::GetLength(firstDifference);
        FLOAT_TYPE secondDifferenceLength = VectorUtilities::GetLength(secondDifference);

        Assert::AreAlmostEqual(0.0, firstDifferenceLength, "FillVelocitiesAfterCollision_FirstParticleMovingAlongXSecondStill_VelocitiesSwitch");
        Assert::AreAlmostEqual(0.0, secondDifferenceLength, "FillVelocitiesAfterCollision_FirstParticleMovingAlongXSecondStill_VelocitiesSwitch");
        TearDown();
    }

    void ParticleCollisionServiceTests::RunTests()
    {
        GetCollisionTime_FirstParticleMovingAlongXSecondStill_TimeCorrect();
        GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowLinear_TimeCorrect();
        GetCollisionTime_FirstParticleMovingAlongXSecondStillParticlesGrowQuadratic_TimeCorrect();
        FillVelocitiesAfterCollision_FirstParticleMovingAlongXSecondStill_VelocitiesSwitch();
        FillVelocitiesAfterCollision_ParticlesStillButGrowing_VelocitiesSpreadThem();
    }
}


