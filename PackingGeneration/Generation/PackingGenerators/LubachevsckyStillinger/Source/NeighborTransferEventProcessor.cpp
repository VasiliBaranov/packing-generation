// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/NeighborTransferEventProcessor.h"
#include "../Headers/IEventProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    NeighborTransferEventProcessor::NeighborTransferEventProcessor(IEventProvider* eventProvider,
            INeighborProvider* neighborProvider,
            Packing* particles) : BaseEventProcessor(eventProvider)
    {
        this->neighborProvider = neighborProvider;
        this->particles = particles;
    }

    void NeighborTransferEventProcessor::ProcessEventSafe(Event event)
    {
        ParticleIndex particleIndex = event.particleIndex;
        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        Packing& particlesRef = *particles;

        neighborProvider->StartMove(particleIndex);
        particlesRef[particleIndex].coordinates = movingParticlesRef[particleIndex].coordinates;
        neighborProvider->EndMove();

        eventProvider->SetNextEvents(movingParticles, event);
    }

    bool NeighborTransferEventProcessor::ShouldProcessEvent(Event event) const
    {
        return event.type == EventType::NeighborTransfer;
    }
}

