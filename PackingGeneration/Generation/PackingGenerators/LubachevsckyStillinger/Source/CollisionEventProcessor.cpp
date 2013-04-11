// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CollisionEventProcessor.h"
#include <map>
#include "Core/Headers/StlUtilities.h"
#include "../Headers/ParticleCollisionService.h"
#include "../Headers/IEventProvider.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    CollisionEventProcessor::CollisionEventProcessor(IEventProvider* eventProvider,
                    ParticleCollisionService* particleCollisionService) : BaseEventProcessor(eventProvider)
    {
        this->particleCollisionService = particleCollisionService;
    }

    void CollisionEventProcessor::ProcessEventSafe(Event event)
    {
        std::vector<MovingParticle>& movingParticlesRef = *movingParticles;

        // Get new velocities of colliding pair
        MovingParticle& particle = movingParticlesRef[event.particleIndex];
        MovingParticle& neighbor = movingParticlesRef[event.neighborIndex];

        SpatialVector particleVelocity;
        SpatialVector neighborVelocity;
        exchangedMomentum += particleCollisionService->FillVelocitiesAfterCollision(currentTime, particle, neighbor, &particleVelocity, &neighborVelocity);

        particle.velocity = particleVelocity;
        neighbor.velocity = neighborVelocity;

        eventProvider->SetNextEvents(movingParticles, event);

        Event symmetricEvent = event;
        symmetricEvent.particleIndex = event.neighborIndex;
        symmetricEvent.neighborIndex = event.particleIndex;
        eventProvider->SetNextEvents(movingParticles, symmetricEvent);

//        collisions.push_back(event);
    }

    bool CollisionEventProcessor::ShouldProcessEvent(Event event) const
    {
        return event.type == EventType::Collision;
    }

    void CollisionEventProcessor::ResetStatistics(int maxEventsPerCycle)
    {
        exchangedMomentum = 0;

        collisions.clear();
        collisions.reserve(maxEventsPerCycle);
    }

    void CollisionEventProcessor::FillDistinctCollidingPairs(std::vector<CollidingPair>* distinctCollidingPairs)
    {
        map<CollidingPair, int> collisionCountsByPairs;

        for (vector<Event>::const_iterator it = collisions.begin(); it != collisions.end(); ++it)
        {
            const Event& collision = *it;
            CollidingPair pair;
            pair.particleIndex = collision.particleIndex;
            pair.neighborIndex = collision.neighborIndex;

            // A new entry will be added, if there were no such collision, with default value (0)
            collisionCountsByPairs[pair]++;

            CollidingPair symmetricPair;
            symmetricPair.neighborIndex = collision.particleIndex;
            symmetricPair.particleIndex = collision.neighborIndex;
            collisionCountsByPairs[symmetricPair]++;
        }

        distinctCollidingPairs->clear();
        for (map<CollidingPair, int>::const_iterator it = collisionCountsByPairs.begin(); it != collisionCountsByPairs.end(); ++it)
        {
            const pair<const CollidingPair, int>& pair = *it;
            CollidingPair collidingPair = pair.first;
            collidingPair.collisionsCount = pair.second;

            distinctCollidingPairs->push_back(collidingPair);
        }
        StlUtilities::Sort(distinctCollidingPairs);
    }
}

