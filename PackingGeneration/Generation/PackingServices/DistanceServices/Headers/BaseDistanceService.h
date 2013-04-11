// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_BaseDistanceService_h
#define Generation_PackingServices_DistanceServices_Headers_BaseDistanceService_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class MathService; }
namespace Model { class SystemConfig; }
namespace Geometries { class IGeometry; }

namespace PackingServices
{
    // A base class for distance calculation methods, a wrapper over neighbor provider.
    class BaseDistanceService : public virtual IContextDependentService
    {
    protected:
        const Model::Packing* particles;
        const Model::SystemConfig* config;
        const Geometries::IGeometry* geometry;

        INeighborProvider* neighborProvider;
        MathService* mathService;

    public:
        BaseDistanceService(MathService* mathService, INeighborProvider* neighborProvider);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        INeighborProvider* GetNeighborProvider() const;

        virtual ~BaseDistanceService() {};
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_BaseDistanceService_h */

