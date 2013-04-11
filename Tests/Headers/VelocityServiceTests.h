// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_VelocityServiceTests_h
#define Headers_VelocityServiceTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"
namespace PackingGenerators { class VelocityService; }

namespace Tests
{
    class VelocityServiceTests
    {
    private:
        static boost::shared_ptr<PackingGenerators::VelocityService> velocityService;
        static std::vector<PackingGenerators::MovingParticle> particles;
        static const int particlesCount;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void FillInitialVelocity_CompareTemperature_ActualTemperatureCorrect();

        static void RescaleVelocities_ToGetTemperature_ActualTemperatureCorrect();
    };
}

#endif /* Headers_VelocityServiceTests_h */
