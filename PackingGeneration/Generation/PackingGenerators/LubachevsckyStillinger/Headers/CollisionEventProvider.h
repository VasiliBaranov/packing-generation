// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProvider_h

#include "BaseEventProvider.h"
namespace PackingGenerators { class ParticleCollisionService; }
namespace PackingServices { class INeighborProvider; }

namespace PackingGenerators
{
    class CollisionEventProvider : public BaseEventProvider
    {
    private:
        ParticleCollisionService* particleCollisionService;
        PackingServices::INeighborProvider* neighborProvider;

    public:
        CollisionEventProvider(ParticleCollisionService* particleCollisionService,
                PackingServices::INeighborProvider* neighborProvider);

        ~CollisionEventProvider() {};

    protected:
        OVERRIDE void SetNextEventsSafe(Event triggerEvent);

    private:
        DISALLOW_COPY_AND_ASSIGN(CollisionEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_CollisionEventProvider_h */
