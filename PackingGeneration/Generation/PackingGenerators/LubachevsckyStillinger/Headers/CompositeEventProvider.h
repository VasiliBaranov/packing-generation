// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProvider_h

#include "BaseEventProvider.h"
namespace Core { template<class TContainer, class TCompare> class OrderedPriorityQueue; }

namespace PackingGenerators
{
    class CompositeEventProvider : public BaseEventProvider
    {
    private:
        Core::OrderedPriorityQueue<std::vector<MovingParticle>, MovingParticleComparer>* eventsQueue;

    public:
        const std::vector<IEventProvider*>* eventProviders;

    public:
        CompositeEventProvider(Core::OrderedPriorityQueue<std::vector<MovingParticle>, MovingParticleComparer>* eventsQueue,
                const std::vector<IEventProvider*>* eventProviders);

        ~CompositeEventProvider() {};

        static void OptimizeEventProviders(std::vector<IEventProvider*>* eventProviders);

    protected:
        OVERRIDE void SetNextEventsSafe(Event triggerEvent);

    private:
        static void RemoveRedundandtProviders(std::vector<IEventProvider*>* eventProviders);

        static void MoveCollisionsProviderToStart(std::vector<IEventProvider*>* eventProviders);

        DISALLOW_COPY_AND_ASSIGN(CompositeEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_CompositeEventProvider_h */
