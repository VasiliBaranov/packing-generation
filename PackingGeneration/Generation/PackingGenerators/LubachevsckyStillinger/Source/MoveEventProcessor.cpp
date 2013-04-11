// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MoveEventProcessor.h"
#include "../Headers/IEventProvider.h"

using namespace Core;
using namespace Model;

namespace PackingGenerators
{
    MoveEventProcessor::MoveEventProcessor(IEventProvider* eventProvider) : BaseEventProcessor(eventProvider)
    {
    }

    void MoveEventProcessor::ProcessEventSafe(Event event)
    {
        // Do nothing. Even if the particle is not synchronized with the current time, it's ok, as nothing happens during the move event.
        // Moreover, particles are synchronized inside the BaseEventProcessor.

        eventProvider->SetNextEvents(movingParticles, event);
    }

    bool MoveEventProcessor::ShouldProcessEvent(Event event) const
    {
        return event.type == EventType::Move;
    }
}

