// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_GeometryCollisionServiceTests_h
#define Headers_GeometryCollisionServiceTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/PackingServices//Headers/GeometryCollisionService.h"

namespace PackingServices { class GeometryCollisionService; }

namespace Tests
{
    class GeometryCollisionServiceTests
    {
    private:
        static boost::shared_ptr<PackingServices::GeometryCollisionService> collisionService;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void GetTransferTime_ParticleMovingAlongX_TimeCorrect();
    };
}

#endif /* Headers_GeometryCollisionServiceTests_h */
