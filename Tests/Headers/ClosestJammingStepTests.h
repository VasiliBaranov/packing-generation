// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_ClosestJammingStepTests_h
#define Headers_ClosestJammingStepTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/Model/Headers/Types.h"
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { class GeometryCollisionService; }
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class IClosestPairProvider; }
namespace PackingGenerators { class ClosestJammingStep; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }
namespace Model { class GenerationConfig; }

namespace Tests
{
    class ClosestJammingStepTests
    {
    private:
        static boost::shared_ptr<PackingServices::GeometryService> geometryService;
        static boost::shared_ptr<PackingServices::GeometryCollisionService> geometryCollisionService;
        static boost::shared_ptr<PackingServices::INeighborProvider> neighborProvider;
        static boost::shared_ptr<PackingServices::IClosestPairProvider> closestPairProvider;
        static boost::shared_ptr<PackingServices::MathService> mathService;
        static boost::shared_ptr<PackingGenerators::ClosestJammingStep> closestJammingStep;

        static boost::shared_ptr<Geometries::IGeometry> geometry;
        static boost::shared_ptr<Model::SystemConfig> config;
        static boost::shared_ptr<Model::ModellingContext> context;
        static boost::shared_ptr<Model::GenerationConfig> generationConfig;
        static Model::Packing particles;

        static const int particlesCount;
        static Core::SpatialVector boxSize;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void DisplaceParticles_ThreeParticlesInContact_DisplacementsCorrect(Core::FLOAT_TYPE timeStep);

        static void DisplaceParticles_FourParticlesInContact_DisplacementsCorrect(Core::FLOAT_TYPE timeStep);

        static void DisplaceParticles_FourParticlesInContactForLargeTime_NoGapsNoIntersections();

        static void DisplaceParticles_ThreeParticlesUntilReachingFourth_NoIntersectionsNoGaps();
    };
}

#endif /* Headers_ClosestJammingStepTests_h */
