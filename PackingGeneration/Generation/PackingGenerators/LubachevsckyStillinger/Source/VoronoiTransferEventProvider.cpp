// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VoronoiTransferEventProvider.h"

#include "../Headers/VelocityService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    VoronoiTransferEventProvider::VoronoiTransferEventProvider(GeometryCollisionService* geometryCollisionService,
            VelocityService* velocityService,
            PackingServices::MathService* mathService,
            vector<VoronoiPolytope>* voronoiTesselation) : BaseEventProvider()
    {
        this->velocityService = velocityService;
        this->geometryCollisionService = geometryCollisionService;
        this->mathService = mathService;
        this->voronoiTesselation = voronoiTesselation;

        reflectFromSpheres = false;
    }

    void VoronoiTransferEventProvider::SetNextEventsSafe(Event triggerEvent)
    {
        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        MovingParticle& particle = movingParticlesRef[triggerEvent.particleIndex];
        VoronoiPolytope& polytope = voronoiTesselation->at(triggerEvent.particleIndex);

        if (!reflectFromSpheres)
        {
            if (triggerEvent.type == EventType::VoronoiInscribedSphereTransfer)
            {
                // Particle is on the boundary of the inscribed sphere.
                SetPolytopeTransferEvent(polytope, -1, &particle);
            }

            if (triggerEvent.type == EventType::VoronoiTransfer)
            {
                // Particle is the boundary of the polytope and is definetely outside the inscribed sphere, but finite precision errors may lead to opposite answer, if sphere is very close to the polytope.
                // We shall exclude the current wall from event search to avoid infinite loops
                SetPolytopeTransferEvent(polytope, triggerEvent.wallIndex, &particle);
            }

            FLOAT_TYPE distanceSquareToInscribedSphereCenter = mathService->GetDistanceSquare(particle.coordinates, polytope.insribedSphereCenter);
            bool particleOutsideInscribedSphere = distanceSquareToInscribedSphereCenter > polytope.insribedSphereRadius * polytope.insribedSphereRadius;

            if (particleOutsideInscribedSphere)
            {
                // Particles should always be inside the polytope, though (because they are reflected from the boundaries)
                SetPolytopeTransferEvent(polytope, -1, &particle);
            }
            else
            {
                SetInscribedSphereTransferEvent(polytope, &particle);
            }
        }
        else
        {
            // VoronoiInscribedSphereTransfer will never be triggered; if trigger event is VoronoiTransfer, we lie on the sphere.
            // This method is called after the event is processed, so the particle is already directed inside the sphere (todo: remove this hidden dependency)
            if (triggerEvent.type == EventType::VoronoiTransfer)
            {
                // We would like to move it slightly further the sphere to avoid infinite loops when searching for the next sphere transfer event
                currentTime += 1e-10;
                velocityService->SynchronizeParticleWithCurrentTime(currentTime, &particle);
            }

            // The particles will always be inside the sphere, because they are reflected from the boundaries
            SetInscribedSphereTransferEvent(polytope, &particle);
            if (particle.nextAvailableEvent.type == EventType::VoronoiInscribedSphereTransfer)
            {
                particle.nextAvailableEvent.type = EventType::VoronoiTransfer;
            }
        }
    }

    void VoronoiTransferEventProvider::SetInscribedSphereTransferEvent(const VoronoiPolytope& polytope, MovingParticle* particle)
    {
        SpatialVector particlePeriodicImage;
        mathService->FillClosestPeriodicImagePosition(polytope.insribedSphereCenter, particle->coordinates, &particlePeriodicImage);
        FLOAT_TYPE intersectionTime = geometryCollisionService->GetSphereIntersectionTime(particlePeriodicImage, particle->velocity, polytope.insribedSphereCenter, polytope.insribedSphereRadius);
        intersectionTime += currentTime;

        bool eventIsValid = intersectionTime >= currentTime;
        bool eventIsEarlierThanFromOtherProviders = intersectionTime < particle->nextAvailableEvent.time;
        if (eventIsValid && eventIsEarlierThanFromOtherProviders)
        {
            Event event = Event::Invalid;
            event.particleIndex = particle->index;
            event.type = EventType::VoronoiInscribedSphereTransfer;
            event.time = intersectionTime;
            particle->nextAvailableEvent = event;
        }
    }

    void VoronoiTransferEventProvider::SetPolytopeTransferEvent(const VoronoiPolytope& polytope, int wallIndexToExclude, MovingParticle* particle)
    {
        FLOAT_TYPE intersectionTime = 0.0;
        int planeIndex = 0;
        geometryCollisionService->FindIntersection(particle->coordinates, particle->velocity, polytope, wallIndexToExclude, &intersectionTime, &planeIndex);
        intersectionTime += currentTime;

        bool eventIsValid = intersectionTime >= currentTime;
        bool eventIsEarlierThanFromOtherProviders = intersectionTime < particle->nextAvailableEvent.time;
        if (eventIsValid && eventIsEarlierThanFromOtherProviders)
        {
            particle->nextAvailableEvent = Event::Invalid;
            particle->nextAvailableEvent.particleIndex = particle->index;
            particle->nextAvailableEvent.type = EventType::VoronoiTransfer;
            particle->nextAvailableEvent.time = intersectionTime;
            particle->nextAvailableEvent.wallIndex = planeIndex;
        }
    }
}

