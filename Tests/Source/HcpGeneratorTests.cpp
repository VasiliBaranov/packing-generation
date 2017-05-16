// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/HcpGeneratorTests.h"
#include <string>
#include <float.h>
#include "../Headers/Assert.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/ScopedFile.h"
#include "Generation/PackingGenerators/InitialGenerators/Headers/HcpGenerator.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;
using namespace PackingGenerators;

namespace Tests
{
    boost::shared_ptr<MathService> HcpGeneratorTests::mathService;
    boost::shared_ptr<HcpGenerator> HcpGeneratorTests::hcpGenerator;

    boost::shared_ptr<IGeometry> HcpGeneratorTests::geometry;
    boost::shared_ptr<SystemConfig> HcpGeneratorTests::config;
    boost::shared_ptr<ModellingContext> HcpGeneratorTests::context;
    Model::Packing HcpGeneratorTests::particles;

    const FLOAT_TYPE HcpGeneratorTests::diameter = 1.0;

    void HcpGeneratorTests::SetUp()
    {
        hcpGenerator.reset(new HcpGenerator());
        mathService.reset(new MathService());

        config.reset(new SystemConfig());
        HcpGenerator::FillExpectedSize(diameter, &config->packingSize);
        config->particlesCount = HcpGenerator::GetExpectedParticlesCount();
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config.get()));
        context.reset(new ModellingContext(config.get(), geometry.get()));

        mathService->SetContext(*context.get());
        hcpGenerator->SetContext(*context.get());
//        hcpGenerator->SetGenerationConfig(generationConfig);

        particles.resize(config->particlesCount);
        for (ParticleIndex i = 0; i < context->config->particlesCount; i++)
        {
            particles[i].diameter = diameter;
            particles[i].index = i;
        }
    }

    void HcpGeneratorTests::TearDown()
    {
    }

    void HcpGeneratorTests::ArrangePacking_ForHcp_NoParticleIntersections()
    {
        SetUp();

        hcpGenerator->ArrangePacking(&particles);

        FLOAT_TYPE minDistanceSquare = FLT_MAX;
        FLOAT_TYPE currentDistanceSquare = 0;

        for (ParticleIndex i = 0; i < context->config->particlesCount - 1; i++)
        {
            for (ParticleIndex j = i + 1; j < context->config->particlesCount; j++)
            {
                currentDistanceSquare = mathService->GetNormalizedDistanceSquare(i, j, particles);
                if (currentDistanceSquare < minDistanceSquare)
                {
                    minDistanceSquare = currentDistanceSquare;
                }
            }
        }

        FLOAT_TYPE minDistance = sqrt(minDistanceSquare);

        Assert::AreAlmostEqual(minDistance, 1.0, "ArrangePacking_ForHcp_NoParticleIntersections");

        TearDown();
    }

    void HcpGeneratorTests::ArrangePacking_ForHcp_DensityCorrect()
    {
        SetUp();

        FLOAT_TYPE boxVolume = VectorUtilities::GetProduct(context->config->packingSize);
        GeometryService geometryService(mathService.get());
        FLOAT_TYPE particlesVolume = geometryService.GetParticlesVolume(particles, *(context->config)); // context->config->particlesCount * PI * diameter * diameter * diameter / 6.0;

        FLOAT_TYPE density = particlesVolume / boxVolume;
        FLOAT_TYPE expectedDensity = (DIMENSIONS == 3) ? PI / 3.0 / sqrt(2.0) : PI / 2.0 / sqrt(3.0);

        Assert::AreAlmostEqual(expectedDensity, density, "ArrangePacking_ForHcp_DensityCorrect");

        TearDown();
    }

    void HcpGeneratorTests::RunTests()
    {
        ArrangePacking_ForHcp_NoParticleIntersections();
        ArrangePacking_ForHcp_DensityCorrect();
    }
}


