// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BaseEventProvider.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    BaseEventProvider::BaseEventProvider()
    {
    }

    void BaseEventProvider::SetNextEvents(vector<MovingParticle>* movingParticles, Event triggerEvent)
    {
        this->movingParticles = movingParticles;
        this->currentTime = triggerEvent.time;

        SetNextEventsSafe(triggerEvent);
    }
}

