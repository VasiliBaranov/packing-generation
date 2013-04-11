// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProvider_h

#include "BaseEventProvider.h"
namespace PackingServices { class GeometryCollisionService; }
namespace PackingGenerators { class VelocityService; }
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class WallTransferEventProvider : public BaseEventProvider
    {
    private:
        Model::CubicBox box;

        VelocityService* velocityService;
        PackingServices::GeometryCollisionService* geometryCollisionService;

    public:
        WallTransferEventProvider(VelocityService* velocityService,
                PackingServices::GeometryCollisionService* geometryCollisionService,
                const Model::SystemConfig& config);

        ~WallTransferEventProvider() {};

    protected:
        OVERRIDE void SetNextEventsSafe(Event triggerEvent);

    private:
        DISALLOW_COPY_AND_ASSIGN(WallTransferEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_WallTransferEventProvider_h */
