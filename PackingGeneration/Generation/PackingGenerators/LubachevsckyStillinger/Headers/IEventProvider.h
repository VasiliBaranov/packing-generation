// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProvider_h

#include "Types.h"

namespace PackingGenerators
{
    class IEventProvider
    {
    public:
        virtual void SetNextEvents(std::vector<MovingParticle>* movingParticles, Event triggerEvent) = 0;

        virtual ~IEventProvider(){ };
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProvider_h */
