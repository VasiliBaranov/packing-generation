// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProcessor_h

#include "BaseEventProcessor.h"
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class WallTransferEventProcessor : public BaseEventProcessor
    {
    private:
        const Model::SystemConfig* config;
        Model::CubicBox box;

    public:
        WallTransferEventProcessor(IEventProvider* eventProvider,
                const Model::SystemConfig& config);

        ~WallTransferEventProcessor() {};

    protected:
        OVERRIDE void ProcessEventSafe(Event event);

        bool ShouldProcessEvent(Event event) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(WallTransferEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProcessor_h */
