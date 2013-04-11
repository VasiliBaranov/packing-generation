// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/WallTransferEventProvider.h"
#include "../Headers/VelocityService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/Model/Headers/Config.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    WallTransferEventProvider::WallTransferEventProvider(VelocityService* velocityService,
            GeometryCollisionService* geometryCollisionService,
            const Model::SystemConfig& config) : BaseEventProvider()
    {
        this->velocityService = velocityService;
        this->geometryCollisionService = geometryCollisionService;

        const SpatialVector minVertexCoordinates = {{0.0, 0.0, 0.0}};
        box.Initialize(minVertexCoordinates, config.packingSize);
    }

    void WallTransferEventProvider::SetNextEventsSafe(Event triggerEvent)
    {
        int wallIndexToExclude = -1;

        // triggerEvent.wallIndex may be specified also for VoronoiTransfer events, so we should not use triggerEvent.wallIndex then
        if (triggerEvent.type == EventType::WallTransfer)
        {
            // Do not search for collisions with the wall we are currently crossing
            wallIndexToExclude = triggerEvent.wallIndex;
        }

        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        MovingParticle& particle = movingParticlesRef[triggerEvent.particleIndex];

        // TODO: move this code to IGeometry, as well as the code that updates the particle after collision
        // (for periodic boundaries it shifts the particle; for solid boundaries a particle should be reflected).
        FLOAT_TYPE intersectionTime = 0.0;
        int wallIndex = 0;
        geometryCollisionService->FindIntersection(particle.coordinates, particle.velocity, box, wallIndexToExclude, &intersectionTime, &wallIndex);
        intersectionTime += currentTime;

        bool eventIsValid = intersectionTime >= currentTime;
        bool eventIsEarlierThanFromOtherProviders = intersectionTime < particle.nextAvailableEvent.time;
        if (eventIsValid && eventIsEarlierThanFromOtherProviders)
        {
            particle.nextAvailableEvent = Event::Invalid;
            particle.nextAvailableEvent.particleIndex = triggerEvent.particleIndex;
            particle.nextAvailableEvent.type = EventType::WallTransfer;
            particle.nextAvailableEvent.time = intersectionTime;
            particle.nextAvailableEvent.wallIndex = wallIndex;
        }
    }
}

