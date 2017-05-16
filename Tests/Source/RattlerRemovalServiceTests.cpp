// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/RattlerRemovalServiceTests.h"

#include "../Headers/Assert.h"
#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/DistanceServices/Headers/CellListNeighborProvider.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"

#include "Generation/PackingServices/PostProcessing/Headers/RattlerRemovalService.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;

namespace Tests
{
    boost::shared_ptr<MathService> RattlerRemovalServiceTests::mathService;
    boost::shared_ptr<GeometryService> RattlerRemovalServiceTests::geometryService;
    boost::shared_ptr<INeighborProvider> RattlerRemovalServiceTests::neighborProvider;
    boost::shared_ptr<RattlerRemovalService> RattlerRemovalServiceTests::rattlerRemovalService;
    boost::shared_ptr<GeometryCollisionService> RattlerRemovalServiceTests::geometryCollisionService;

    boost::shared_ptr<IGeometry> RattlerRemovalServiceTests::geometry;
    boost::shared_ptr<SystemConfig> RattlerRemovalServiceTests::config;
    boost::shared_ptr<ModellingContext> RattlerRemovalServiceTests::context;

    Packing RattlerRemovalServiceTests::particles;
    int RattlerRemovalServiceTests::particlesCount = 4;
    SpatialVector RattlerRemovalServiceTests::boxSize = REMOVE_LAST_DIMENSION_IF_NEEDED(10.0, 10.0, 10.0);

    void RattlerRemovalServiceTests::SetUp()
    {
        mathService.reset(new MathService());
        geometryService.reset(new GeometryService(mathService.get()));
        geometryCollisionService.reset(new GeometryCollisionService());
        neighborProvider.reset(new CellListNeighborProvider(geometryService.get(), geometryCollisionService.get()));
        rattlerRemovalService.reset(new RattlerRemovalService(mathService.get(), neighborProvider.get()));

        config.reset(new SystemConfig());
        config->packingSize = boxSize;
        config->particlesCount = particlesCount;
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config));

        context.reset(new ModellingContext(config.get(), geometry.get()));

        mathService->SetContext(*context);
        geometryService->SetContext(*context);
        neighborProvider->SetContext(*context);
        rattlerRemovalService->SetContext(*context);
        rattlerRemovalService->SetMinNeighborsCount(1);

        particles.resize(particlesCount);
    }

    void RattlerRemovalServiceTests::TearDown()
    {
    }

    void RattlerRemovalServiceTests::RemoveRattlers_ForDensePacking_NoParticlesRemoved()
    {
        SetUp();

        // All particles contact each other
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(4, 4, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(4, 5, 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 4, 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        rattlerRemovalService->SetParticles(particles);

        vector<bool> rattlerMask(particlesCount);
        rattlerRemovalService->FillRattlerMask(0.999, &rattlerMask);

        boost::array<bool, 4> expectedRattlerMask = {{false, false, false, false}};
        Assert::AreVectorsEqual(expectedRattlerMask, rattlerMask, "RemoveRattlers_ForDensePacking_NoParticlesRemoved");

        TearDown();
    }

    void RattlerRemovalServiceTests::RemoveRattlers_ForMixedPacking_CorrectParticlesRemoved()
    {
        SetUp();

        // The second particle is outside
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(4, 4, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(7, 7, 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 4, 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        rattlerRemovalService->SetParticles(particles);

        vector<bool> rattlerMask(particlesCount);
        rattlerRemovalService->FillRattlerMask(0.999, &rattlerMask);

        boost::array<bool, 4> expectedRattlerMask = {{false, true, false, false}};
        Assert::AreVectorsEqual(expectedRattlerMask, rattlerMask, "RemoveRattlers_ForMixedPacking_CorrectParticlesRemoved");

        TearDown();
    }

    void RattlerRemovalServiceTests::RemoveRattlers_ForLoosePacking_AllParticlesRemoved()
    {
        SetUp();

        // Diameter of each particle is very small
        const FLOAT_TYPE diameter = 0.1;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(4, 4, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(4, 5, 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 4, 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        rattlerRemovalService->SetParticles(particles);

        vector<bool> rattlerMask(particlesCount);
        rattlerRemovalService->FillRattlerMask(0.999, &rattlerMask);

        boost::array<bool, 4> expectedRattlerMask = {{true, true, true, true}};
        Assert::AreVectorsEqual(expectedRattlerMask, rattlerMask, "RemoveRattlers_ForLoosePacking_AllParticlesRemoved");

        TearDown();
    }

    void RattlerRemovalServiceTests::RunTests()
    {
        RemoveRattlers_ForDensePacking_NoParticlesRemoved();
        RemoveRattlers_ForMixedPacking_CorrectParticlesRemoved();
        RemoveRattlers_ForLoosePacking_AllParticlesRemoved();
    }
}


