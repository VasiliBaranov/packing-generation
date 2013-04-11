// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/NeighborTransferEventProvider.h"
#include "../Headers/VelocityService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    NeighborTransferEventProvider::NeighborTransferEventProvider(PackingServices::INeighborProvider* neighborProvider,
            VelocityService* velocityService) : BaseEventProvider()
    {
        this->neighborProvider = neighborProvider;
        this->velocityService = velocityService;
    }

    void NeighborTransferEventProvider::SetNextEventsSafe(Event triggerEvent)
    {
        if (triggerEvent.type == EventType::NeighborTransfer)
        {
            // The particle is currently lying on the neighbor provider update boundary,
            // and we would like to move it slightly outside to avoid infinite loops when searching for the next neighbor transfer event
            currentTime += 1e-10;
        }

        ParticleIndex particleIndex = triggerEvent.particleIndex;
        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        MovingParticle* particle = &movingParticlesRef[particleIndex];

        FLOAT_TYPE updateBoundaryTime = neighborProvider->GetTimeToUpdateBoundary(particleIndex, particle->coordinates, particle->velocity);
        updateBoundaryTime += currentTime;

        bool eventIsValid = updateBoundaryTime >= currentTime;
        bool eventIsEarlierThanFromOtherProviders = updateBoundaryTime < particle->nextAvailableEvent.time;
        if (eventIsValid && eventIsEarlierThanFromOtherProviders)
        {
            particle->nextAvailableEvent = Event::Invalid;
            particle->nextAvailableEvent.particleIndex = particleIndex;
            particle->nextAvailableEvent.type = EventType::NeighborTransfer;
            particle->nextAvailableEvent.time = updateBoundaryTime;
        }
    }
}

