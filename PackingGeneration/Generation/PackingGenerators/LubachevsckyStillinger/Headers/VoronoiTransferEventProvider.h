// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProvider_h

#include "BaseEventProvider.h"
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryCollisionService; }
namespace PackingGenerators { class VelocityService; }

namespace PackingGenerators
{
    class VoronoiTransferEventProvider : public BaseEventProvider
    {
    private:
        PackingServices::GeometryCollisionService* geometryCollisionService;
        VelocityService* velocityService;
        PackingServices::MathService* mathService;

    public:
        std::vector<VoronoiPolytope>* voronoiTesselation;
        bool reflectFromSpheres;

    public:
        VoronoiTransferEventProvider(PackingServices::GeometryCollisionService* geometryCollisionService,
                VelocityService* velocityService,
                PackingServices::MathService* mathService,
                std::vector<VoronoiPolytope>* voronoiTesselation);

        ~VoronoiTransferEventProvider() {};

    protected:
        OVERRIDE void SetNextEventsSafe(Event triggerEvent);

    private:
        void SetInscribedSphereTransferEvent(const VoronoiPolytope& polytope, MovingParticle* particle);

        void SetPolytopeTransferEvent(const VoronoiPolytope& polytope, int wallIndexToExclude, MovingParticle* particle);

        DISALLOW_COPY_AND_ASSIGN(VoronoiTransferEventProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTransferEventProvider_h */
