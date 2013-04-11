// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/WallTransferEventProcessor.h"
#include "Generation/Model/Headers/Config.h"
#include "../Headers/IEventProvider.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    WallTransferEventProcessor::WallTransferEventProcessor(IEventProvider* eventProvider,
            const Model::SystemConfig& config) : BaseEventProcessor(eventProvider)
    {
        this->config = &config;

        const SpatialVector minVertexCoordinates = {{0.0, 0.0, 0.0}};
        box.Initialize(minVertexCoordinates, config.packingSize);
    }

    void WallTransferEventProcessor::ProcessEventSafe(Event event)
    {
        ParticleIndex particleIndex = event.particleIndex;
        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        MovingParticle* particle = &movingParticlesRef[particleIndex];
        SimplePlane* wall = &box.walls[event.wallIndex];

        particle->coordinates[wall->perpendicularAxis] -= wall->outerNormalDirection * config->packingSize[wall->perpendicularAxis];
        eventProvider->SetNextEvents(movingParticles, event);
    }

    bool WallTransferEventProcessor::ShouldProcessEvent(Event event) const
    {
        return event.type == EventType::WallTransfer;
    }
}

