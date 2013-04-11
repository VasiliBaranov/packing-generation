// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProcessor_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProcessor_h

#include "BaseEventProcessor.h"

namespace PackingGenerators
{
    class VoronoiTransferEventProcessor : public BaseEventProcessor
    {
    public:
        std::vector<VoronoiPolytope>* voronoiTesselation;
        bool reflectFromSpheres;

    public:
        VoronoiTransferEventProcessor(IEventProvider* eventProvider,
                std::vector<VoronoiPolytope>* voronoiTesselation);

        ~VoronoiTransferEventProcessor() {};

    protected:
        OVERRIDE void ProcessEventSafe(Event event);

        bool ShouldProcessEvent(Event event) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(VoronoiTransferEventProcessor);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProcessor_h */
