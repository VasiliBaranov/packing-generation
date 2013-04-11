// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_MoveEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_MoveEventProcessor_h

#include "BaseEventProcessor.h"

namespace PackingGenerators
{
    class MoveEventProcessor : public BaseEventProcessor
    {
    public:
        MoveEventProcessor(IEventProvider* eventProvider);

        ~MoveEventProcessor() {};

    protected:
        OVERRIDE void ProcessEventSafe(Event event);

        bool ShouldProcessEvent(Event event) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(MoveEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_MoveEventProcessor_h */
