// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProcessor_h

#include "BaseEventProcessor.h"
namespace PackingServices { class INeighborProvider; }

namespace PackingGenerators
{
    class NeighborTransferEventProcessor : public BaseEventProcessor
    {
    private:
        Model::Packing* particles;
        PackingServices::INeighborProvider* neighborProvider;

    public:
        NeighborTransferEventProcessor(IEventProvider* eventProvider,
                PackingServices::INeighborProvider* neighborProvider,
                Model::Packing* particles);

        ~NeighborTransferEventProcessor() {};

    protected:
        OVERRIDE void ProcessEventSafe(Event event);

        bool ShouldProcessEvent(Event event) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(NeighborTransferEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProcessor_h */
