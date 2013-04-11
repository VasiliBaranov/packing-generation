// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_ClosestPairProviderTests_h
#define Headers_ClosestPairProviderTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/Model/Headers/Types.h"
namespace Core { class MemoryUtility; }
namespace PackingServices { class GeometryCollisionService; }
namespace PackingServices { class ClosestPairProvider; }
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { class INeighborProvider; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace Tests
{
    class ClosestPairProviderTests
    {
    private:
        static boost::shared_ptr<PackingServices::ClosestPairProvider> closestPairProvider;
        static boost::shared_ptr<Core::MemoryUtility> memoryService;
        static boost::shared_ptr<PackingServices::MathService> mathService;
        static boost::shared_ptr<PackingServices::GeometryService> geometryService;
        static boost::shared_ptr<PackingServices::GeometryCollisionService> geometryCollisionService;
        static boost::shared_ptr<PackingServices::INeighborProvider> neighborProvider;

        static boost::shared_ptr<Geometries::IGeometry> geometry;
        static boost::shared_ptr<Model::SystemConfig> config;
        static boost::shared_ptr<Model::ModellingContext> context;
        static Model::Packing particles;
    public:
        static void RunTests();
    private:
        static void SetUp();
        static void TearDown();

        static void AssertPair(Model::ParticlePair expectedPair, Model::ParticlePair actualPair, std::string functionName);

        static void GetNearestNeighbor_ForThreeParticles_ReturnNearest();
        static void GetNearestNeighbor_ForThreeParticlesWithPeriodicPair_ReturnNearest();
        static void UnregisterPair_ForFourParticles_LeftParticlesAreNearest();
        static void RegisterPair_ForFourParticlesAndMovedParticlesAreAway_NonMovedParticlesAreNearest();
        static void RegisterPair_ForFourParticlesAndMovedParticlesAreClose_MovedParticlesAreNearest();
        static void RegisterPair_ForFourParticlesAndMovedParticleIsCloseToNonMoved_ThisPairIsReturned();
        static void RegisterPair_ForFourParticlesAndMovedParticlesAreCloseThroughPeriodic_MovedParticlesAreNearest();
    };
}

#endif /* Headers_ClosestPairProviderTests_h */
