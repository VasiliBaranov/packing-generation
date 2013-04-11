// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ClosestPairProvider.h"

#include "Core/Headers/Constants.h"
#include "../Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace Model;
using namespace Core;
using namespace std;

namespace PackingServices
{
    ClosestPairProvider::ClosestPairProvider(MathService* mathService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {
    }

    ClosestPairProvider::~ClosestPairProvider()
    {
    }

    void ClosestPairProvider::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);

        particlesWithNeighbors.clear();
        particlesWithNeighbors.resize(config->particlesCount);
        Particle::CopyPackingTo(particles, &particlesWithNeighbors);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            FillClosestNeighbor(&particlesWithNeighbors[particleIndex], -1);
        }

        neighborDistancesQueue.Initialize(&particlesWithNeighbors, ParticleWithNeighborComparer());
    }

    ParticlePair ClosestPairProvider::FindClosestPair() const
    {
        ParticleIndex particleIndex = neighborDistancesQueue.GetTopIndex();
        const ParticleWithNeighbor* closestParticle = &particlesWithNeighbors[particleIndex];

        ParticlePair pair(closestParticle->index, closestParticle->closestNeighborIndex, closestParticle->closestNormalizedDistanceSquare);
        return pair;
    }

    ParticlePair ClosestPairProvider::FindClosestNeighbor(ParticleIndex particleIndex) const
    {
        const ParticleWithNeighbor* particle = &particlesWithNeighbors[particleIndex];
        ParticlePair pair(particleIndex, particle->closestNeighborIndex, particle->closestNormalizedDistanceSquare);

        return pair;
    }

    void ClosestPairProvider::StartMove(ParticleIndex particleIndex)
    {
        movedParticleIndex = particleIndex;

        ParticleWithNeighbor* particle = &particlesWithNeighbors[particleIndex];
        RemoveParticleFromNeighbors(particle);

        // Internal services StartMove should be the last, so that previous service usages are valid.
        neighborProvider->StartMove(particleIndex);
    }

    void ClosestPairProvider::EndMove()
    {
        // Internal services EndMove should be the first, so that following service usages are valid.
        neighborProvider->EndMove();

        const Packing& particlesRef = *particles;
        particlesWithNeighbors[movedParticleIndex].coordinates = particlesRef[movedParticleIndex].coordinates;
        ParticleWithNeighbor* particle = &particlesWithNeighbors[movedParticleIndex];

        AddParticleToNeighbors(particle);
    }

    void ClosestPairProvider::FillClosestNeighbor(ParticleWithNeighbor* particle, ParticleIndex indexToExclude)
    {
        FLOAT_TYPE minDistanceSquare;
        ParticleIndex closestNeighborIndex = -1;

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particle->index, &neighborsCount);

        minDistanceSquare = MAX_FLOAT_VALUE;

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            if (neighborIndex != indexToExclude)
            {
                FLOAT_TYPE distanceSquare = mathService->GetNormalizedDistanceSquare(*particle, particlesWithNeighbors[neighborIndex]);

                if (distanceSquare < minDistanceSquare)
                {
                    minDistanceSquare = distanceSquare;
                    closestNeighborIndex = neighborIndex;
                }
            }
        }

        particle->closestNeighborIndex = closestNeighborIndex;
        particle->closestNormalizedDistanceSquare = minDistanceSquare;
    }

    // TODO: Merge with FillClosestNeighbor
    void ClosestPairProvider::AddParticleToNeighbors(ParticleWithNeighbor* particle)
    {
        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particle->index, &neighborsCount);

        FLOAT_TYPE minDistanceSquare = MAX_FLOAT_VALUE;
        unsigned int closestNeighborIndex = -1;

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            ParticleWithNeighbor* neighbor = &particlesWithNeighbors[neighborIndex];
            // Wish there were good anonymous functions earlier than C++ 11
            // then all the code above could have been extracted and reused in several functions.

            FLOAT_TYPE currentDistanceSquare = mathService->GetNormalizedDistanceSquare(*particle, *neighbor);

            if (currentDistanceSquare < minDistanceSquare)
            {
                minDistanceSquare = currentDistanceSquare;
                closestNeighborIndex = neighbor->index;
            }

            if (neighbor->closestNormalizedDistanceSquare > currentDistanceSquare)
            {
                neighbor->closestNormalizedDistanceSquare = currentDistanceSquare;
                neighbor->closestNeighborIndex = particle->index;
                neighborDistancesQueue.HandleUpdate(neighbor->index);
            }
        }

        particle->closestNormalizedDistanceSquare = minDistanceSquare;
        particle->closestNeighborIndex = closestNeighborIndex;
        neighborDistancesQueue.HandleUpdate(particle->index);
    }

    void ClosestPairProvider::RemoveParticleFromNeighbors(ParticleWithNeighbor* particle)
    {
        //Check each neighbor of the particle.
        //If the particle A has been the closest neighbor for B, remove B from the lists, update nearest neighbor for B
        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particle->index, &neighborsCount);

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            ParticleWithNeighbor* neighbor = &particlesWithNeighbors[neighborIndex];

            if (neighbor->closestNeighborIndex == particle->index)
            {
                FillClosestNeighbor(neighbor, particle->index);
                neighborDistancesQueue.HandleUpdate(neighbor->index);
            }
        }

        neighborDistancesQueue.HandleUpdate(particle->index);
    }
}

