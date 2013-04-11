// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CollisionEventProvider.h"
#include "Core/Headers/Constants.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "../Headers/ParticleCollisionService.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    CollisionEventProvider::CollisionEventProvider(ParticleCollisionService* particleCollisionService,
            INeighborProvider* neighborProvider) : BaseEventProvider()
    {
        this->particleCollisionService = particleCollisionService;
        this->neighborProvider = neighborProvider;
    }

    void CollisionEventProvider::SetNextEventsSafe(Event triggerEvent)
    {
        vector<MovingParticle>& movingParticlesRef = *movingParticles;

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(triggerEvent.particleIndex, &neighborsCount);

        FLOAT_TYPE collisionTime = MAX_FLOAT_VALUE;
        int collidingNeighborIndex = -1;

        MovingParticle& particle = movingParticlesRef[triggerEvent.particleIndex];
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            if (neighborIndex == triggerEvent.neighborIndex)
            {
                continue;
            }

            MovingParticle& neighbor = movingParticlesRef[neighborIndex];
            FLOAT_TYPE currentCollisionTime = particleCollisionService->GetCollisionTime(currentTime, particle, neighbor);

            // If spheres were also overlapping due to machine precision errors, the returned collision time may be equal to the current time
            bool collisionIsValid = currentCollisionTime >= currentTime;
            bool neighborIsAvailable = currentCollisionTime <= neighbor.nextAvailableEvent.time;
            bool availableEventIsEarlier = currentCollisionTime < collisionTime;
            if (collisionIsValid && neighborIsAvailable && availableEventIsEarlier)
            {
                collisionTime = currentCollisionTime;
                collidingNeighborIndex = neighbor.index;
            }
        }

        bool eventIsValid = collidingNeighborIndex >= 0;
        bool eventIsEarlierThanFromOtherProviders = collisionTime < particle.nextAvailableEvent.time;
        if (eventIsValid && eventIsEarlierThanFromOtherProviders)
        {
            particle.nextAvailableEvent = Event::Invalid;
            particle.nextAvailableEvent.type = EventType::Collision;
            particle.nextAvailableEvent.particleIndex = triggerEvent.particleIndex;
            particle.nextAvailableEvent.neighborIndex = collidingNeighborIndex;
            particle.nextAvailableEvent.time = collisionTime;
        }
//        else
//        {
//            printf("WARNING: No colliding neighbors found for %d at time %f.\n", particleIndex, time);
//        }
    }
}

