// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/GeometryService.h"

#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "../Headers/MathService.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    GeometryService::GeometryService(MathService* mathService)
    {
        this->mathService = mathService;
    }

    void GeometryService::SetContext(const ModellingContext& context)
    {
        this->config = context.config;
        mathService->SetContext(context);
    }

    GeometryService::~GeometryService()
    {

    }

    FLOAT_TYPE GeometryService::GetParticlesVolume(const Packing& particles) const
    {
        return GetParticlesVolume(particles, *config);
    }

    FLOAT_TYPE GeometryService::GetParticlesVolume(const Packing& particles, const SystemConfig& config) const
    {
        FLOAT_TYPE particlesVolume = 0.0;
        for (ParticleIndex i = 0; i < config.particlesCount; ++i)
        {
            particlesVolume += (PI * particles[i].diameter * particles[i].diameter * particles[i].diameter / 6.0);
        }

        return particlesVolume;
    }

    FLOAT_TYPE GeometryService::GetPorosity(const Packing& particles, const SystemConfig& config) const
    {
        return 1.0 - GetParticlesVolume(particles, config) / VectorUtilities::GetProduct(config.packingSize);
    }

    FLOAT_TYPE GeometryService::GetMaxParticleDiameter(const Packing& particles) const
    {
        FLOAT_TYPE maxDiameter = 0.0;
        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            if (particles[i].diameter > maxDiameter)
            {
                maxDiameter = particles[i].diameter;
            }
        }

        return maxDiameter;
    }

    FLOAT_TYPE GeometryService::GetMinParticleDiameter(const Packing& particles) const
    {
        FLOAT_TYPE minDiameter = MAX_FLOAT_VALUE;
        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            if (particles[i].diameter < minDiameter)
            {
                minDiameter = particles[i].diameter;
            }
        }

        return minDiameter;
    }

    FLOAT_TYPE GeometryService::GetMeanParticleDiameter(const Packing& particles) const
    {
        FLOAT_TYPE meanDiameter = 0.0;
        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            meanDiameter += particles[i].diameter;
        }

        return meanDiameter / config->particlesCount;
    }

    void GeometryService::ScaleDiameters(Packing* particles, Core::FLOAT_TYPE scalingFactor) const
    {
        Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DomainParticle& particle = particlesRef[particleIndex];
            particle.diameter = scalingFactor * particle.diameter;
        }
    }

    FLOAT_TYPE GeometryService::GetScalingFactor(Core::FLOAT_TYPE currentDensity, Core::FLOAT_TYPE targetDensity) const
    {
        double contractionRate = pow(currentDensity / targetDensity, 1.0 / 3.0);
        return contractionRate;
    }

    FLOAT_TYPE GeometryService::GetScalingFactorByPorosity(Core::FLOAT_TYPE currentPorosity, Core::FLOAT_TYPE targetPorosity) const
    {
        return GetScalingFactor(1.0 - currentPorosity, 1.0 - targetPorosity);
    }
}

