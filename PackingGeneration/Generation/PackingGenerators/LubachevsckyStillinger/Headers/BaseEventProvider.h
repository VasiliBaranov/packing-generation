// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProvider_h

#include "Core/Headers/Macros.h"
#include "IEventProvider.h"

namespace PackingGenerators
{
    class BaseEventProvider : public virtual IEventProvider
    {
    protected:
        // Working variables
        Core::FLOAT_TYPE currentTime;
        std::vector<MovingParticle>* movingParticles;

    public:
        BaseEventProvider();

        OVERRIDE void SetNextEvents(std::vector<MovingParticle>* movingParticles, Event triggerEvent);

        ~BaseEventProvider() {};

    protected:
        virtual void SetNextEventsSafe(Event triggerEvent) = 0;

    private:
        DISALLOW_COPY_AND_ASSIGN(BaseEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProvider_h */

