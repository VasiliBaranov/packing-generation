// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CompositeEventProvider.h"

#include "Core/Headers/OrderedPriorityQueue.h"
#include "Core/Headers/StlUtilities.h"
#include "../Headers/VoronoiTransferEventProvider.h"
#include "../Headers/NeighborTransferEventProvider.h"
#include "../Headers/CollisionEventProvider.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    CompositeEventProvider::CompositeEventProvider(OrderedPriorityQueue<std::vector<MovingParticle>, MovingParticleComparer>* eventsQueue,
            const vector<IEventProvider*>* eventProviders) : BaseEventProvider()
    {
        this->eventsQueue = eventsQueue;
        this->eventProviders = eventProviders;
    }

    void CompositeEventProvider::OptimizeEventProviders(vector<IEventProvider*>* eventProviders)
    {
        RemoveRedundandtProviders(eventProviders);
        MoveCollisionsProviderToStart(eventProviders);
    }

    void CompositeEventProvider::RemoveRedundandtProviders(vector<IEventProvider*>* eventProviders)
    {
        // Remove redundant NeighborTransferEventProvider, if VoronoiTransferEventProvider reflects from spheres
        VoronoiTransferEventProvider* voronoiTransferEventProvider = StlUtilities::FindObject<IEventProvider, VoronoiTransferEventProvider>(*eventProviders);
        if (voronoiTransferEventProvider == NULL || !voronoiTransferEventProvider->reflectFromSpheres)
        {
            return;
        }

        // TODO: remove assumptions!
        // Assume that they are equal for all the spheres, if reflectFromSpheres option is true
        FLOAT_TYPE inscribedSphereRadii = voronoiTransferEventProvider->voronoiTesselation->at(0).insribedSphereRadius;

        // Assume that Verlet list neighbor provider is active, and its cutoff distance is larger or equal than 0.5
        bool verletTransferIsRedundant = inscribedSphereRadii <= 0.5;
        if (verletTransferIsRedundant)
        {
            // Remove it, so NeighborTransfer events are never issued
            StlUtilities::RemoveObject<IEventProvider, VoronoiTransferEventProvider>(eventProviders);
        }
    }

    void CompositeEventProvider::MoveCollisionsProviderToStart(vector<IEventProvider*>* eventProviders)
    {
        CollisionEventProvider* collisionEventProvider = StlUtilities::RemoveObject<IEventProvider, CollisionEventProvider>(eventProviders);
        if (collisionEventProvider == NULL)
        {
            return;
        }

        eventProviders->insert(eventProviders->begin(), collisionEventProvider);
    }

    void CompositeEventProvider::SetNextEventsSafe(Event triggerEvent)
    {
        vector<MovingParticle>& movingParticlesRef = *movingParticles;

        MovingParticle& particle = movingParticlesRef[triggerEvent.particleIndex];
        particle.nextAvailableEvent = Event::Invalid;
        // As far as eventsQueue->HandleUpdate will never be called in event providers below, we may postpone this call.
//         eventsQueue->HandleUpdate(triggerEvent.particleIndex);

        for (vector<IEventProvider*>::const_iterator it = eventProviders->begin(); it != eventProviders->end(); ++it)
        {
            IEventProvider* eventProvider = *it;
            eventProvider->SetNextEvents(movingParticles, triggerEvent);
        }

        eventsQueue->HandleUpdate(triggerEvent.particleIndex);
        if (particle.nextAvailableEvent.neighborIndex == Event::InvalidIndex)
        {
            return;
        }

        MovingParticle& neighbor = movingParticlesRef[particle.nextAvailableEvent.neighborIndex];

        // Reset the event of neighbor's neighbor
        if (neighbor.nextAvailableEvent.neighborIndex != Event::InvalidIndex && neighbor.nextAvailableEvent.neighborIndex != triggerEvent.particleIndex)
        {
            MovingParticle& neighborsNeighbor = movingParticlesRef[neighbor.nextAvailableEvent.neighborIndex];
            neighborsNeighbor.nextAvailableEvent.neighborIndex = Event::InvalidIndex;
            neighborsNeighbor.nextAvailableEvent.type = EventType::Move;
//             eventsQueue->HandleUpdate(neighborsNeighbor.index); // this call is redundant, as event time is not changed
        }

        neighbor.nextAvailableEvent = particle.nextAvailableEvent;
        neighbor.nextAvailableEvent.particleIndex = neighbor.index;
        neighbor.nextAvailableEvent.neighborIndex = particle.index;
        eventsQueue->HandleUpdate(neighbor.index); // this call is not redundant, as neighbor event time has changed
    }
}

