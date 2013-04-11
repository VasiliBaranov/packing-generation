// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProcessor_h

#include "Types.h"

namespace PackingGenerators
{
    class IEventProcessor
    {
    public:
        virtual void ProcessEvent(std::vector<MovingParticle>* movingParticles, Event event) = 0;

        virtual ~IEventProcessor(){ };
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_IEventProcessor_h */
