// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProvider_h

#include "BaseEventProvider.h"
namespace PackingServices { class INeighborProvider; }
namespace PackingGenerators { class VelocityService; }

namespace PackingGenerators
{
    class NeighborTransferEventProvider : public BaseEventProvider
    {
    private:
        PackingServices::INeighborProvider* neighborProvider;
        VelocityService* velocityService;

    public:
        NeighborTransferEventProvider(PackingServices::INeighborProvider* neighborProvider,
                VelocityService* velocityService);

        ~NeighborTransferEventProvider() {};

    protected:
        OVERRIDE void SetNextEventsSafe(Event triggerEvent);

    private:
        DISALLOW_COPY_AND_ASSIGN(NeighborTransferEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_NeighborTransferEventProvider_h */
