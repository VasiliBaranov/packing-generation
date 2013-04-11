// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProcessor_h

#include <map>
#include "Core/Headers/Macros.h"
#include "IEventProcessor.h"
namespace PackingGenerators { class VelocityService; }

namespace PackingGenerators
{
    class CompositeEventProcessor : public virtual IEventProcessor
    {
    private:
        std::map<EventType::Type, int> eventsStatistics;
        VelocityService* velocityService;

    public:
        const std::vector<IEventProcessor*>* eventProcessors;

    public:
        CompositeEventProcessor(const std::vector<IEventProcessor*>& eventProcessors, VelocityService* velocityService);

        OVERRIDE void ProcessEvent(std::vector<MovingParticle>* movingParticles, Event event);

        void ResetStatistics();

        int GetEventTypeCount(EventType::Type type);

        ~CompositeEventProcessor() {};

    private:
        DISALLOW_COPY_AND_ASSIGN(CompositeEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProcessor_h */

