// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BaseEventProcessor.h"
#include "../Headers/IEventProvider.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    BaseEventProcessor::BaseEventProcessor(IEventProvider* eventProvider)
    {
        this->eventProvider = eventProvider;
    }

    void BaseEventProcessor::ProcessEvent(vector<MovingParticle>* movingParticles, Event event)
    {
        if (!ShouldProcessEvent(event))
        {
            return;
        }

        this->movingParticles = movingParticles;
        this->currentTime = event.time;

        ProcessEventSafe(event);
    }
}

