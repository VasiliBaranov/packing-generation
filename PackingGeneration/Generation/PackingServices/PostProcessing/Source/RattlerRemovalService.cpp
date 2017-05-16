// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/RattlerRemovalService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    RattlerRemovalService::RattlerRemovalService(MathService* mathService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {
        minNeighborsCount = 0;
    }

    void RattlerRemovalService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    int RattlerRemovalService::GetMinNeighborsCount() const
    {
        return minNeighborsCount;
    }

    void RattlerRemovalService::SetMinNeighborsCount(int value)
    {
        minNeighborsCount = value;
    }

    void RattlerRemovalService::FillRattlerMask(FLOAT_TYPE contractionRatio, vector<bool>* rattlerMask) const
    {
        vector<bool> processedMask(config->particlesCount, false);
        rattlerMask->resize(config->particlesCount, false);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            bool processed = processedMask[particleIndex];
            if (!processed)
            {
                CheckIfRattler(particleIndex, contractionRatio, rattlerMask, &processedMask);
            }
        }
    }

    int RattlerRemovalService::FindNonRattlersCount(const vector<bool>& rattlerMask) const
    {
        int nonRattlersCount = 0;
        for (int i = 0; i < config->particlesCount; ++i)
        {
            if (!rattlerMask[i])
            {
                nonRattlersCount++;
            }
        }
        return nonRattlersCount;
    }

    void RattlerRemovalService::FillNonRattlerPacking(const vector<bool>& rattlerMask, Packing* nonRattlerParticles) const
    {
        const Packing& particlesRef = *particles;
        Packing& nonRattlerParticlesRef = *nonRattlerParticles;
        int nonRattlerIndex = 0;
        for (int i = 0; i < config->particlesCount; ++i)
        {
            if (!rattlerMask[i])
            {
                particlesRef[i].CopyTo(&nonRattlerParticlesRef[nonRattlerIndex]);
                nonRattlerParticlesRef[nonRattlerIndex].index = nonRattlerIndex;
                nonRattlerIndex++;
            }
        }
    }

    void RattlerRemovalService::FillNonRattlerConfig(int nonRattlersCount, const ExecutionConfig& oldConfig, ExecutionConfig* newConfig) const
    {
        newConfig->MergeWith(oldConfig);
        newConfig->systemConfig.particlesCount = nonRattlersCount;
    }

    void RattlerRemovalService::FillNonRattlerPackingInfo(int nonRattlersCount, const Packing& nonRattlerParticles, const ExecutionConfig& newConfig, const PackingInfo& oldInfo, PackingInfo* newInfo) const
    {
        // A dirty hack to call a default copy constructor
        newInfo->operator=(oldInfo);
        // (*newInfo).operator=(*oldInfo);

        GeometryService geometryService(mathService);
        newInfo->calculatedPorosity = geometryService.GetPorosity(nonRattlerParticles, newConfig.systemConfig);
    }

    void RattlerRemovalService::CheckIfRattler(ParticleIndex particleIndex, FLOAT_TYPE contractionRatio, vector<bool>* rattlerMask, vector<bool>* processedMask) const
    {
        vector<bool>& rattlerMaskRef = *rattlerMask;
        vector<bool>& processedMaskRef = *processedMask;
        const Packing& particlesRef = *particles;

        if (particlesRef[particleIndex].isImmobile)
        {
            processedMaskRef[particleIndex] = true;
            rattlerMaskRef[particleIndex] = false;
            return;
        }

        // Continue, even if processed and not rattler, as may call when removing an intersecting particle, and introduce new rattlers
        if (processedMaskRef[particleIndex] && rattlerMaskRef[particleIndex])
        {
            return;
        }

        const DomainParticle& particle = particlesRef[particleIndex];

        // Find intersecting non-rattler particles
        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
        vector<ParticleIndex> intersectingNeighborIndexes;
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle& neighbor = particlesRef[neighborIndex];

            if (processedMaskRef[neighborIndex] && rattlerMaskRef[neighborIndex])
            {
                continue;
            }

            FLOAT_TYPE diameterRatioSquare =  mathService->GetNormalizedDistanceSquare(neighbor, particle);

            FLOAT_TYPE contractedDiameterRatioSquare = diameterRatioSquare * contractionRatio * contractionRatio;
            if (contractedDiameterRatioSquare >= 1.0)
            {
                continue;
            }

            intersectingNeighborIndexes.push_back(neighborIndex);
        }

        // Mark the current one as processed and as (non)rattler
        bool isRattler = intersectingNeighborIndexes.size() < static_cast<size_t>(minNeighborsCount);
        processedMaskRef[particleIndex] = true;
        rattlerMaskRef[particleIndex] = isRattler;

        // If become a rattler, for each intersecting non-rattler call CheckIfRattler
        if (isRattler)
        {
            for (vector<ParticleIndex>::size_type i = 0; i < intersectingNeighborIndexes.size(); ++i)
            {
                ParticleIndex neighborIndex = intersectingNeighborIndexes[i];
                CheckIfRattler(neighborIndex, contractionRatio, rattlerMask, processedMask);
            }
        }
    }
}

