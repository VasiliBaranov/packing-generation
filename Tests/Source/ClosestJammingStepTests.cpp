// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ClosestJammingStepTests.h"

#include "../Headers/Assert.h"
#include "Core/Headers/Math.h"
#include "Core/Headers/MemoryUtility.h"
#include "Core/Headers/StlUtilities.h"
#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/Model/Headers/Config.h"

#include "Generation/PackingServices/DistanceServices/Headers/CellListNeighborProvider.h"

#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/DistanceServices/Headers/ClosestPairProvider.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"

#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingServices/PostProcessing/Headers/HessianService.h"
#include "Generation/PackingServices/PostProcessing/Headers/RattlerRemovalService.h"

#include "Generation/PackingGenerators/Headers/ClosestJammingStep.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;
using namespace PackingGenerators;

namespace Tests
{
    boost::shared_ptr<MathService> ClosestJammingStepTests::mathService;
    boost::shared_ptr<GeometryService> ClosestJammingStepTests::geometryService;
    boost::shared_ptr<INeighborProvider> ClosestJammingStepTests::neighborProvider;
    boost::shared_ptr<IClosestPairProvider> ClosestJammingStepTests::closestPairProvider;
    boost::shared_ptr<GeometryCollisionService> ClosestJammingStepTests::geometryCollisionService;

    boost::shared_ptr<ClosestJammingStep> ClosestJammingStepTests::closestJammingStep;

    boost::shared_ptr<IGeometry> ClosestJammingStepTests::geometry;
    boost::shared_ptr<SystemConfig> ClosestJammingStepTests::config;
    boost::shared_ptr<ModellingContext> ClosestJammingStepTests::context;
    boost::shared_ptr<GenerationConfig> ClosestJammingStepTests::generationConfig;

    Packing ClosestJammingStepTests::particles;
    const int ClosestJammingStepTests::particlesCount = 4;
    SpatialVector ClosestJammingStepTests::boxSize = REMOVE_LAST_DIMENSION_IF_NEEDED(10.0, 10.0, 10.0);

    void ClosestJammingStepTests::SetUp()
    {
        mathService.reset(new MathService());
        geometryService.reset(new GeometryService(mathService.get()));
        geometryCollisionService.reset(new GeometryCollisionService());
        neighborProvider.reset(new CellListNeighborProvider(geometryService.get(), geometryCollisionService.get()));
        closestPairProvider.reset(new ClosestPairProvider(mathService.get(), neighborProvider.get()));
        closestJammingStep.reset(new ClosestJammingStep(geometryService.get(), neighborProvider.get(), closestPairProvider.get(), mathService.get()));

        config.reset(new SystemConfig());
        config->packingSize = boxSize;
        config->particlesCount = particlesCount;
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config));
        context.reset(new ModellingContext(config.get(), geometry.get()));
        generationConfig.reset(new GenerationConfig());

        mathService->SetContext(*context);
        geometryService->SetContext(*context);
        neighborProvider->SetContext(*context);
        closestPairProvider->SetContext(*context);
        closestJammingStep->SetContext(*context);

        closestJammingStep->SetGenerationConfig(*generationConfig);

        particles.clear();
        particles.resize(particlesCount);
    }

    void ClosestJammingStepTests::TearDown()
    {
    }

    void ClosestJammingStepTests::DisplaceParticles_ThreeParticlesInContact_DisplacementsCorrect(FLOAT_TYPE timeStep)
    {
        SetUp();
        closestJammingStep->maxTimeStep = timeStep;
        closestJammingStep->SetBondThreshold(1e-10);
        closestJammingStep->integrationTimeStep = 1e-4;

        // Arrange

        // Three particles form a regular triangle, the fourth is away
        const FLOAT_TYPE diameter = 0.5; // not 1.0, to test non-trivial diameters
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(diameter, 0, 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(diameter * 0.5, diameter * sin(PI / 3.0), 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        boost::array<SpatialVector, particlesCount> initialPositions = {{c0, c1, c2, c3}};

        // Act

        closestJammingStep->SetParticles(&particles);
        closestJammingStep->DisplaceParticles();

        // Assert

        // Prepare expected displacements
        // From-to
        SpatialVector c01, c12, c02;
        mathService->FillDistance(c1, c0, &c01);
        mathService->FillDistance(c2, c1, &c12);
        mathService->FillDistance(c2, c0, &c02);

        SpatialVector d0, d1, d2;

        VectorUtilities::Add(c01, c02, &d0);
        VectorUtilities::MultiplyByValue(d0, - 1.0 / 3.0 * timeStep, &d0);

        VectorUtilities::Subtract(c12, c01, &d1);
        VectorUtilities::MultiplyByValue(d1, - 1.0 / 3.0 * timeStep, &d1);

        VectorUtilities::Add(c12, c02, &d2);
        VectorUtilities::MultiplyByValue(d2, 1.0 / 3.0 * timeStep, &d2);

        SpatialVector d3 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);

        boost::array<SpatialVector, particlesCount> expectedDisplacements = {{d0, d1, d2, d3}};

        // Get actual displacements
        vector<SpatialVector> actualDisplacements(particlesCount);
        for (int i = 0; i < particlesCount; ++i)
        {
            mathService->FillDistance(particles[i].coordinates, initialPositions[i], &actualDisplacements[i]);
        }

        // Compare displacements
        for (int i = 0; i < particlesCount; ++i)
        {
//            printf("Index %d, actualDisplacement %g %g %g, expectedDisplacement %g %g %g\n",
//                    i,
//                    actualDisplacements[i][0], actualDisplacements[i][1], actualDisplacements[i][2],
//                    expectedDisplacements[i][0], expectedDisplacements[i][1], expectedDisplacements[i][2]);
            Assert::AreVectorsAlmostEqual(actualDisplacements[i], expectedDisplacements[i], "DisplaceParticles_ThreeParticlesInContact_DisplacementsCorrect");
        }

        TearDown();
    }

    void ClosestJammingStepTests::DisplaceParticles_FourParticlesInContact_DisplacementsCorrect(FLOAT_TYPE timeStep)
    {
        SetUp();
        closestJammingStep->maxTimeStep = timeStep;
        closestJammingStep->SetBondThreshold(1e-10);
        closestJammingStep->integrationTimeStep = 1e-4;

        // Arrange

        // Particles form a "snake"
        const FLOAT_TYPE diameter = 0.5; // not 1.0, to test non-trivial diameters
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(c0[0] + diameter * cos(PI / 3.0), c0[1] + diameter * sin(PI / 3.0), 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(c1[0] + diameter, c1[1], 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(c2[0], c2[1] + diameter, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        boost::array<SpatialVector, particlesCount> initialPositions = {{c0, c1, c2, c3}};

        // Act

        closestJammingStep->SetParticles(&particles);
        closestJammingStep->DisplaceParticles();

        // Assert

        // Prepare expected displacements
        // From-to
        SpatialVector c01, c12, c23;
        mathService->FillDistance(c1, c0, &c01);
        mathService->FillDistance(c2, c1, &c12);
        mathService->FillDistance(c3, c2, &c23);

        SpatialVector d0, d1, d2, d3;

        // Displacements calculated on paper
        VectorUtilities::MultiplyByValue(c01, - 2.0 / 3.0 * timeStep, &d0);

        VectorUtilities::Subtract(c01, c12, &d1);
        VectorUtilities::MultiplyByValue(d1, 2.0 / 3.0 * timeStep, &d1);

        VectorUtilities::MultiplyByValue(c12, 2.0 / 3.0 * timeStep, &d2);
        SpatialVector temp;
        VectorUtilities::MultiplyByValue(c23, - 0.5 * timeStep, &temp);
        VectorUtilities::Add(d2, temp, &d2);

        VectorUtilities::MultiplyByValue(c23, 0.5 * timeStep, &d3);

        boost::array<SpatialVector, particlesCount> expectedDisplacements = {{d0, d1, d2, d3}};

        // Get actual displacements
        vector<SpatialVector> actualDisplacements(particlesCount);
        for (int i = 0; i < particlesCount; ++i)
        {
            mathService->FillDistance(particles[i].coordinates, initialPositions[i], &actualDisplacements[i]);
        }

        // Compare displacements
        for (int i = 0; i < particlesCount; ++i)
        {
//            printf("Index %d, actualDisplacement %g %g %g, expectedDisplacement %g %g %g\n",
//                    i,
//                    actualDisplacements[i][0], actualDisplacements[i][1], actualDisplacements[i][2],
//                    expectedDisplacements[i][0], expectedDisplacements[i][1], expectedDisplacements[i][2]);
            Assert::AreVectorsAlmostEqual(actualDisplacements[i], expectedDisplacements[i], "DisplaceParticles_FourParticlesInContact_DisplacementsCorrect");
        }

        TearDown();
    }

    void ClosestJammingStepTests::DisplaceParticles_FourParticlesInContactForLargeTime_NoGapsNoIntersections()
    {
        SetUp();
        const FLOAT_TYPE timeStep = 0.001;
        closestJammingStep->maxTimeStep = timeStep;
        closestJammingStep->SetBondThreshold(1e-10);
        closestJammingStep->integrationTimeStep = 1e-7;

        // Arrange

        // Particles form a "snake"
        const FLOAT_TYPE diameter = 0.5; // not 1.0, to test non-trivial diameters
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(c0[0] + diameter * cos(PI / 3.0), c0[1] + diameter * sin(PI / 3.0), 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(c1[0] + diameter, c1[1], 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(c2[0], c2[1] + diameter, 0);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        // Act

        closestJammingStep->SetParticles(&particles);
        closestJammingStep->DisplaceParticles();

        // Assert

        // No intersections
        for (int i = 0; i < particlesCount - 1; ++i)
        {
            for (int j = i + 1; j < particlesCount; ++j)
            {
                FLOAT_TYPE distanceSquare = mathService->GetDistanceSquare(particles[i].coordinates, particles[j].coordinates);
                const FLOAT_TYPE errorThreshold = 1e-12;
                FLOAT_TYPE minDistance = (particles[i].diameter + particles[j].diameter) * 0.5 * (1.0 + timeStep) * (1.0 - errorThreshold);
                bool hasIntersection = (distanceSquare < minDistance * minDistance);
                Assert::IsTrue(!hasIntersection, "DisplaceParticles_FourParticlesInContactForLargeTime_NoGapsNoIntersections");
            }
        }

        // No gaps
        boost::array< pair<int, int>, 3> particlePairs = {{ pair<int, int>(0, 1), pair<int, int>(1, 2), pair<int, int>(2, 3) }};
        for (size_t i = 0; i < particlePairs.size(); ++i)
        {
            const pair<int, int>& pair = particlePairs[i];
            FLOAT_TYPE distanceSquare = mathService->GetDistanceSquare(particles[pair.first].coordinates, particles[pair.second].coordinates);
            const FLOAT_TYPE errorThreshold = 1e-10;
            FLOAT_TYPE maxDistance = (particles[pair.first].diameter + particles[pair.second].diameter) * 0.5 * (1.0 + timeStep) * (1.0 + errorThreshold);
            bool hasGap = (distanceSquare > maxDistance * maxDistance);
            Assert::IsTrue(!hasGap, "DisplaceParticles_FourParticlesInContactForLargeTime_NoGapsNoIntersections");
        }

        TearDown();
    }

    void ClosestJammingStepTests::DisplaceParticles_ThreeParticlesUntilReachingFourth_NoIntersectionsNoGaps()
    {
        SetUp();
        closestJammingStep->SetBondThreshold(1e-10);
        closestJammingStep->integrationTimeStep = 1e-7;

        // Arrange

        // Three particles form a regular triangle, the fourth is away
        const FLOAT_TYPE diameter = 0.5; // not 1.0, to test non-trivial diameters
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(0, 0, 0);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(diameter, 0, 0);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(diameter * 0.5, diameter * sin(PI / 3.0), 0);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(diameter * 0.5, diameter * sin(PI / 3.0) + diameter + 0.001, 0); // When the third particle will contact this one, the growth will stop
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);

        // Act

        closestJammingStep->SetParticles(&particles);
        closestJammingStep->DisplaceParticles();

        // Assert

        // Get the time step dynamically
        FLOAT_TYPE currentTime = mathService->GetNormalizedDistance(particles[2], particles[3]);

        // No intersections
        for (int i = 0; i < particlesCount - 1; ++i)
        {
            for (int j = i + 1; j < particlesCount; ++j)
            {
                FLOAT_TYPE distanceSquare = mathService->GetDistanceSquare(particles[i].coordinates, particles[j].coordinates);
                const FLOAT_TYPE errorThreshold = 1e-10;
                FLOAT_TYPE minDistance = (particles[i].diameter + particles[j].diameter) * 0.5 * currentTime * (1.0 - errorThreshold);
                bool hasIntersection = (distanceSquare < minDistance * minDistance);
                Assert::IsTrue(!hasIntersection, "DisplaceParticles_ThreeParticlesUntilReachingFourth_NoIntersectionsNoGaps");
            }
        }

        // No gaps
        boost::array< pair<int, int>, 3> particlePairs = {{ pair<int, int>(0, 1), pair<int, int>(1, 2), pair<int, int>(2, 3) }};
        for (size_t i = 0; i < particlePairs.size(); ++i)
        {
            const pair<int, int>& pair = particlePairs[i];
            FLOAT_TYPE distanceSquare = mathService->GetDistanceSquare(particles[pair.first].coordinates, particles[pair.second].coordinates);
            const FLOAT_TYPE errorThreshold = 1e-10;
            FLOAT_TYPE maxDistance = (particles[pair.first].diameter + particles[pair.second].diameter) * 0.5 * currentTime * (1.0 + errorThreshold);
            bool hasGap = (distanceSquare > maxDistance * maxDistance);
            Assert::IsTrue(!hasGap, "DisplaceParticles_ThreeParticlesUntilReachingFourth_NoIntersectionsNoGaps");
        }

        TearDown();
    }

    void ClosestJammingStepTests::RunTests()
    {
        DisplaceParticles_ThreeParticlesInContact_DisplacementsCorrect(1e-8);
        DisplaceParticles_ThreeParticlesInContact_DisplacementsCorrect(0.1);

        DisplaceParticles_FourParticlesInContact_DisplacementsCorrect(1e-8);
        DisplaceParticles_FourParticlesInContactForLargeTime_NoGapsNoIntersections();

        DisplaceParticles_ThreeParticlesUntilReachingFourth_NoIntersectionsNoGaps();
    }
}


