// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProcessor_h

#include "Core/Headers/Macros.h"
#include "IEventProcessor.h"
namespace PackingGenerators { class IEventProvider; }

namespace PackingGenerators
{
    class BaseEventProcessor : public virtual IEventProcessor
    {
    protected:
        // Working variables
        Core::FLOAT_TYPE currentTime;
        std::vector<MovingParticle>* movingParticles;

        IEventProvider* eventProvider;

    public:
        BaseEventProcessor(IEventProvider* eventProvider);

        OVERRIDE void ProcessEvent(std::vector<MovingParticle>* movingParticles, Event event);

        ~BaseEventProcessor() {};

    protected:
        virtual void ProcessEventSafe(Event event) = 0;

        virtual bool ShouldProcessEvent(Event event) const = 0;

    private:
        DISALLOW_COPY_AND_ASSIGN(BaseEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_BaseEventProcessor_h */

