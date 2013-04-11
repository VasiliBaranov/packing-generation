// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/HcpGenerator.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Exceptions.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;

namespace PackingGenerators
{
    HcpGenerator::HcpGenerator()
    {
    }

    void HcpGenerator::SetContext(const ModellingContext& context)
    {
        this->context = &context;
        this->config = context.config;
    }

    void HcpGenerator::SetGenerationConfig(const GenerationConfig& generationConfig)
    {
    }

    void HcpGenerator::ArrangePacking(Packing* particles)
    {
        this->particles = particles;
        currentParticleIndex = 0;
        Packing& particlesRef = *particles;
        radius = particlesRef[0].diameter * 0.5;

        Check();

        FLOAT_TYPE heightBetweenLayers = radius * 2.0 * sqrt(6.0) / 3.0;
        FLOAT_TYPE layerBShiftByY = radius / sqrt(3.0);

        SpatialVector firstParticleCenter = {{0.0, 0.0, 0.0}};
        AddLayerA(&firstParticleCenter);

        firstParticleCenter[0] = radius;
        firstParticleCenter[1] = layerBShiftByY;
        firstParticleCenter[2] = heightBetweenLayers;
        AddLayerB(&firstParticleCenter);

        firstParticleCenter[0] = 0.0;
        firstParticleCenter[1] = 0.0;
        firstParticleCenter[2] = 2.0 * heightBetweenLayers;
        AddLayerA(&firstParticleCenter);

        firstParticleCenter[0] = radius;
        firstParticleCenter[1] = layerBShiftByY;
        firstParticleCenter[2] = 3.0 * heightBetweenLayers;
        AddLayerB(&firstParticleCenter);
    }

    FLOAT_TYPE HcpGenerator::GetFinalInnerDiameterRatio() const
    {
        return 0.0;
    }

    void HcpGenerator::AddRowAlongX(const SpatialVector& firstParticleCenter)
    {
        Packing& particlesRef = *particles;
        for (int i = 0; i < 3; ++i)
        {
            DomainParticle& particle = particlesRef[currentParticleIndex + i];
            particle.coordinates = firstParticleCenter;
            particle.coordinates[0] += i * 2.0 * radius;
        }

        currentParticleIndex += 3;
    }

    void HcpGenerator::AddLayerA(SpatialVector* firstParticleCenter)
    {
        AddLayer(firstParticleCenter, radius);
    }

    void HcpGenerator::AddLayerB(SpatialVector* firstParticleCenter)
    {
        AddLayer(firstParticleCenter, -radius);
    }

    void HcpGenerator::AddLayer(SpatialVector* firstParticleCenter, FLOAT_TYPE rowShiftByX)
    {
        FLOAT_TYPE rowShiftByY = radius * sqrt(3.0); // 2.0 * radius * sin(60)
        SpatialVector& firstParticleCenterRef = *firstParticleCenter;

        AddRowAlongX(firstParticleCenterRef);

        firstParticleCenterRef[Axis::X] += rowShiftByX;
        firstParticleCenterRef[Axis::Y] += rowShiftByY;

        AddRowAlongX(firstParticleCenterRef);

        firstParticleCenterRef[Axis::X] -= rowShiftByX; // Minus is correct
        firstParticleCenterRef[Axis::Y] += rowShiftByY;

        AddRowAlongX(firstParticleCenterRef);

        firstParticleCenterRef[Axis::X] += rowShiftByX;
        firstParticleCenterRef[Axis::Y] += rowShiftByY;

        AddRowAlongX(firstParticleCenterRef);
    }

    void HcpGenerator::Check() const
    {
        if (this->config->particlesCount != expectedParticlesCount)
        {
            throw NotImplementedException("Currently support just a periodic packing of 48 particles.");
        }

        SpatialVector expectedSize;
        HcpGenerator::FillExpectedSize(particles->at(0).diameter, &expectedSize);
        SpatialVector sizeRatio;
        VectorUtilities::Divide(this->config->packingSize, expectedSize, &sizeRatio);

        bool sizeCorrect = true;

        for (int i = 0; i < DIMENSIONS; ++i)
        {
            if (std::abs(sizeRatio[i] - 1.0) > 1e-10)
            {
                sizeCorrect = false;
                break;
            }
        }

        if (!sizeCorrect)
        {
            throw NotImplementedException("Currently support just a periodic packing of 48 particles.");
        }
    }

    int HcpGenerator::GetExpectedParticlesCount()
    {
        return expectedParticlesCount;
    }

    void HcpGenerator::FillExpectedSize(FLOAT_TYPE particleDiameter, SpatialVector* expectedSize)
    {
        FLOAT_TYPE radius = particleDiameter * 0.5;
        SpatialVector result = {{6.0 * radius, 4.0 * sqrt(3.0) * radius, 8.0 * sqrt(6.0) / 3.0 * radius}};
        *expectedSize = result;
    }

    HcpGenerator::~HcpGenerator()
    {
    }
}

