// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ClosestPairProviderTests.h"

#include "../Headers/Assert.h"
#include "Generation/PackingServices/DistanceServices/Headers/ClosestPairProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/CellListNeighborProvider.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Core/Headers/MemoryUtility.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/Model/Headers/Config.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;

namespace Tests
{
    boost::shared_ptr<ClosestPairProvider> ClosestPairProviderTests::closestPairProvider;
    boost::shared_ptr<MemoryUtility> ClosestPairProviderTests::memoryService;
    boost::shared_ptr<MathService> ClosestPairProviderTests::mathService;
    boost::shared_ptr<GeometryService> ClosestPairProviderTests::geometryService;
    boost::shared_ptr<GeometryCollisionService> ClosestPairProviderTests::geometryCollisionService;
    boost::shared_ptr<INeighborProvider> ClosestPairProviderTests::neighborProvider;

    boost::shared_ptr<IGeometry> ClosestPairProviderTests::geometry;
    boost::shared_ptr<SystemConfig> ClosestPairProviderTests::config;
    boost::shared_ptr<ModellingContext> ClosestPairProviderTests::context;
    Packing ClosestPairProviderTests::particles;

    void ClosestPairProviderTests::SetUp()
    {
        mathService.reset(new MathService());
        geometryService.reset(new GeometryService(mathService.get()));
        geometryCollisionService.reset(new GeometryCollisionService());
        neighborProvider.reset(new CellListNeighborProvider(geometryService.get(), geometryCollisionService.get()));
        closestPairProvider.reset(new ClosestPairProvider(mathService.get(), neighborProvider.get()));

        config.reset(new SystemConfig());
        SpatialVector packignSize = REMOVE_LAST_DIMENSION_IF_NEEDED(10, 10, 10);
        config->packingSize = packignSize;
        config->particlesCount = 4;
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config.get()));
        context.reset(new ModellingContext(config.get(), geometry.get()));

        mathService->SetContext(*context.get());
        geometryService->SetContext(*context.get());
        neighborProvider->SetContext(*context.get());
        closestPairProvider->SetContext(*context.get());

        particles.resize(config->particlesCount);
    }

    void ClosestPairProviderTests::TearDown()
    {
    }

    void ClosestPairProviderTests::AssertPair(ParticlePair expectedPair, ParticlePair actualPair, string functionName)
    {
        printf((functionName + "\n").c_str());

        bool particleCorrect =
                (expectedPair.firstParticleIndex == actualPair.firstParticleIndex && expectedPair.secondParticleIndex == actualPair.secondParticleIndex) ||
                (expectedPair.firstParticleIndex == actualPair.secondParticleIndex && expectedPair.secondParticleIndex == actualPair.firstParticleIndex);
        Assert::IsTrue(particleCorrect, functionName);
        Assert::AreAlmostEqual<FLOAT_TYPE>(expectedPair.normalizedDistanceSquare, actualPair.normalizedDistanceSquare, functionName, 1e-4);
    }

    void ClosestPairProviderTests::GetNearestNeighbor_ForThreeParticles_ReturnNearest()
    {
        SetUp();

        // Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(6, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(2, 3, 0.5 * 0.5);

        AssertPair(expectedPair, actualPair, "GetNearestNeighbor_ForThreeParticles_ReturnNearest");

        TearDown();
    }

    void ClosestPairProviderTests::GetNearestNeighbor_ForThreeParticlesWithPeriodicPair_ReturnNearest()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(6, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(9.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(2, 3, 0.5 * 0.5);

        AssertPair(expectedPair, actualPair, "GetNearestNeighbor_ForThreeParticlesWithPeriodicPair_ReturnNearest");

        TearDown();
    }

    void ClosestPairProviderTests::UnregisterPair_ForFourParticles_LeftParticlesAreNearest()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.9, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        closestPairProvider->StartMove(2);
        closestPairProvider->StartMove(3);
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(0, 1, 0.9 * 0.9);

        AssertPair(expectedPair, actualPair, "UnregisterPair_ForFourParticles_LeftParticlesAreNearest");

        TearDown();
    }

    void ClosestPairProviderTests::RegisterPair_ForFourParticlesAndMovedParticlesAreAway_NonMovedParticlesAreNearest()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.9, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        closestPairProvider->StartMove(3);
        particles[3].coordinates[Axis::X] = 6;
        closestPairProvider->EndMove();
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(0, 1, 0.9 * 0.9);

        AssertPair(expectedPair, actualPair, "RegisterPair_ForFourParticlesAndMovedParticlesAreAway_NonMovedParticlesAreNearest");

        TearDown();
    }

    void ClosestPairProviderTests::RegisterPair_ForFourParticlesAndMovedParticlesAreClose_MovedParticlesAreNearest()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.9, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        closestPairProvider->StartMove(1);
        particles[1].coordinates[Axis::X] = 5.2;
        closestPairProvider->EndMove();
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(0, 1, 0.2 * 0.2);

        AssertPair(expectedPair, actualPair, "RegisterPair_ForFourParticlesAndMovedParticlesAreClose_MovedParticlesAreNearest");

        TearDown();
    }

    void ClosestPairProviderTests::RegisterPair_ForFourParticlesAndMovedParticleIsCloseToNonMoved_ThisPairIsReturned()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);
        closestPairProvider->StartMove(0);
        particles[0].coordinates[Axis::Y] = 7.9;
        closestPairProvider->EndMove();
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(0, 2, 0.1 * 0.1);

        AssertPair(expectedPair, actualPair, "RegisterPair_ForFourParticlesAndMovedParticleIsCloseToNonMoved_ThisPairIsReturned");

        TearDown();
    }

    void ClosestPairProviderTests::RegisterPair_ForFourParticlesAndMovedParticlesAreCloseThroughPeriodic_MovedParticlesAreNearest()
    {
        SetUp();

        //Closest pair is 2-3 with distance 0.5
        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.9, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        closestPairProvider->SetParticles(particles);

        closestPairProvider->StartMove(0);
        particles[0].coordinates[Axis::X] = 0.2;
        closestPairProvider->EndMove();

        closestPairProvider->StartMove(1);
        particles[1].coordinates[Axis::X] = 9.99999;
        closestPairProvider->EndMove();
        ParticlePair actualPair = closestPairProvider->FindClosestPair();
        ParticlePair expectedPair(0, 1, 0.2 * 0.2);

        AssertPair(expectedPair, actualPair, "RegisterPair_ForFourParticlesAndMovedParticlesAreCloseThroughPeriodic_MovedParticlesAreNearest");

        TearDown();
    }

    void ClosestPairProviderTests::RunTests()
    {
        GetNearestNeighbor_ForThreeParticles_ReturnNearest();
        GetNearestNeighbor_ForThreeParticlesWithPeriodicPair_ReturnNearest();
        UnregisterPair_ForFourParticles_LeftParticlesAreNearest();
        RegisterPair_ForFourParticlesAndMovedParticlesAreAway_NonMovedParticlesAreNearest();
        RegisterPair_ForFourParticlesAndMovedParticlesAreClose_MovedParticlesAreNearest();
        RegisterPair_ForFourParticlesAndMovedParticleIsCloseToNonMoved_ThisPairIsReturned();
        RegisterPair_ForFourParticlesAndMovedParticlesAreCloseThroughPeriodic_MovedParticlesAreNearest();
    }
}


