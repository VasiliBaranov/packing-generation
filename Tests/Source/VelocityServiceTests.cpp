// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VelocityServiceTests.h"

#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/VelocityService.h"
#include "../Headers/Assert.h"

using namespace std;
using namespace Core;
using namespace PackingGenerators;

namespace Tests
{
    boost::shared_ptr<VelocityService> VelocityServiceTests::velocityService;
    vector<MovingParticle> VelocityServiceTests::particles;
    const int VelocityServiceTests::particlesCount = 1e4;

    void VelocityServiceTests::SetUp()
    {
        velocityService.reset(new VelocityService());
        particles.resize(particlesCount);
    }

    void VelocityServiceTests::TearDown()
    {
    }

    void VelocityServiceTests::FillInitialVelocity_CompareTemperature_ActualTemperatureCorrect()
    {
        SetUp();

        velocityService->FillVelocities(&particles);

        FLOAT_TYPE actualKineticEnergy = velocityService->GetActualKineticEnergy(particles);
        FLOAT_TYPE expectedKineticEnergy = velocityService->GetExpectedKineticEnergy(particles);

        Assert::AreAlmostEqual(actualKineticEnergy, expectedKineticEnergy, "FillInitialVelocity_CompareTemperature_ActualTemperatureCorrect");
        TearDown();
    }

    void VelocityServiceTests::RescaleVelocities_ToGetTemperature_ActualTemperatureCorrect()
    {
        SetUp();

        velocityService->FillVelocities(&particles);
        for (int i = 0; i < particlesCount; i++)
        {
            particles[i].velocity[0] += 0.5;
        }

        FLOAT_TYPE actualKineticEnergy = velocityService->GetActualKineticEnergy(particles);
        velocityService->RescaleVelocities(0.0, actualKineticEnergy, &particles);

        actualKineticEnergy = velocityService->GetActualKineticEnergy(particles);
        FLOAT_TYPE expectedKineticEnergy = velocityService->GetExpectedKineticEnergy(particles);

        Assert::AreAlmostEqual(actualKineticEnergy, expectedKineticEnergy, "FillInitialVelocity_CompareTemperature_ActualTemperatureCorrect");
        TearDown();
    }

    void VelocityServiceTests::RunTests()
    {
        FillInitialVelocity_CompareTemperature_ActualTemperatureCorrect();
        RescaleVelocities_ToGetTemperature_ActualTemperatureCorrect();
    }
}


