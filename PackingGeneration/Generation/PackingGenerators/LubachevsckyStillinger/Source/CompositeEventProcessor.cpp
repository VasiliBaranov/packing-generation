// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CompositeEventProcessor.h"
#include "../Headers/VelocityService.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    CompositeEventProcessor::CompositeEventProcessor(const vector<IEventProcessor*>& eventProcessors, VelocityService* velocityService)
    {
        this->eventProcessors = &eventProcessors;
        this->velocityService = velocityService;

        vector<EventType> supportedEventTypes;
        ResetStatistics();
    }

    void CompositeEventProcessor::ResetStatistics()
    {
        for (int i = 0; i < EVENT_TYPES_COUNT; i++)
        {
            eventsStatistics[EVENT_TYPES[i]] = 0;
        }
    }

    int CompositeEventProcessor::GetEventTypeCount(EventType::Type type)
    {
        return eventsStatistics[type];
    }

    void CompositeEventProcessor::ProcessEvent(vector<MovingParticle>* movingParticles, Event event)
    {
        // eventsStatistics[event.type]++;

        vector<MovingParticle>& movingParticlesRef = *movingParticles;

        // Synchronization code is moved from BaseEventProcessor to avoid being called for each processor.
        MovingParticle* particle = &movingParticlesRef[event.particleIndex];
        velocityService->SynchronizeParticleWithCurrentTime(event.time, particle);

        if (event.neighborIndex != Event::InvalidIndex)
        {
            MovingParticle* neighbor = &movingParticlesRef[event.neighborIndex];
            velocityService->SynchronizeParticleWithCurrentTime(event.time, neighbor);
        }

        // Currently each event is processed by a single processor, the others simply return;
        // but this doesn't impose overhead:
        // a packing of 10 000 particles at density 0.6 is compressed with the compression rate 1 to reduced pressure 1e12 in 30.809 s with this approach
        // if select a processor from an array by event type index, generation time is 31.185 s.
        for (vector<IEventProcessor*>::const_iterator it = eventProcessors->begin(); it != eventProcessors->end(); ++it)
        {
            IEventProcessor* eventProcessor = *it;
            eventProcessor->ProcessEvent(movingParticles, event);
        }
    }
}

