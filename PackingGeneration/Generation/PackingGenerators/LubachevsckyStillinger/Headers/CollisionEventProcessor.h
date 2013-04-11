// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProcessor_h

#include "BaseEventProcessor.h"
namespace PackingGenerators { class ParticleCollisionService; }

namespace PackingGenerators
{
    class CollisionEventProcessor : public BaseEventProcessor
    {
    private:
        ParticleCollisionService* particleCollisionService;
        std::vector<Event> collisions;

    public:
        Core::FLOAT_TYPE exchangedMomentum;

    public:
        CollisionEventProcessor(IEventProvider* eventProvider,
                ParticleCollisionService* particleCollisionService);

        void ResetStatistics(int maxEventsPerCycle);

        void FillDistinctCollidingPairs(std::vector<CollidingPair>* distinctCollidingPairs);

        ~CollisionEventProcessor() {};

    protected:
        OVERRIDE void ProcessEventSafe(Event event);

        bool ShouldProcessEvent(Event event) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(CollisionEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProcessor_h */
