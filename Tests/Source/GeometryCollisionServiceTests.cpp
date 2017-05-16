// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/GeometryCollisionServiceTests.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "../Headers/Assert.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace Tests
{
    boost::shared_ptr<GeometryCollisionService> GeometryCollisionServiceTests::collisionService;

    void GeometryCollisionServiceTests::SetUp()
    {
        collisionService.reset(new GeometryCollisionService());
    }

    void GeometryCollisionServiceTests::TearDown()
    {
    }

    void GeometryCollisionServiceTests::GetTransferTime_ParticleMovingAlongX_TimeCorrect()
    {
        SetUp();

        SimplePlane wall;
        wall.perpendicularAxis = Axis::X;
        wall.outerNormalDirection = 1;
        wall.coordinateOnAxis = 2;

        SpatialVector velocity = REMOVE_LAST_DIMENSION_IF_NEEDED(1.0, 2.0, 0.0);
        SpatialVector position = REMOVE_LAST_DIMENSION_IF_NEEDED(0.0, 0.0, 0.0);

        FLOAT_TYPE time = collisionService->GetPlaneIntersectionTime(position, velocity, wall);
        FLOAT_TYPE expectedTime = wall.coordinateOnAxis / velocity[Axis::X];

        Assert::AreAlmostEqual(time, expectedTime, "GetTransferTime_ParticleMovingAlongX_TimeCorrect");
        TearDown();
    }

    void GeometryCollisionServiceTests::RunTests()
    {
        GetTransferTime_ParticleMovingAlongX_TimeCorrect();
    }
}


