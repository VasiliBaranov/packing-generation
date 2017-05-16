// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_ImmobileParticlesService_h
#define Generation_PackingServices_Headers_ImmobileParticlesService_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"

namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }

namespace PackingServices { class MathService; }
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class GeometryService; }

namespace PackingServices
{
    class ImmobileParticlesService : public virtual IContextDependentService
    {
    private:
        // Services
        MathService* mathService;
        GeometryService* geometryService;
        INeighborProvider* neighborProvider;

        // Data
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;
        const Model::Packing* particles;

    public:
        ImmobileParticlesService(MathService* mathService, GeometryService* geometryService, INeighborProvider* neighborProvider);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        void SetAndArrangeImmobileParticles(Model::Packing* particles);

        ~ImmobileParticlesService();

    private:
        void FillCellsPerSide(Core::DiscreteSpatialVector* cellsPerSide) const;

        void FillDistancesToCenter(const Model::ParticleIndex* neighborIndexes, Model::ParticleIndex neighborsCount, const Core::SpatialVector& cellCenter, std::vector<Core::FLOAT_TYPE>* distancesToCellCenter) const;

        int GetImmobileNeighborLocalIndex(const std::vector<Core::FLOAT_TYPE>& distancesToCellCenter) const;

        void FillCellCenter(int cellIndex, const Core::DiscreteSpatialVector& cellsPerSide, Core::SpatialVector* cellCenter) const;

        DISALLOW_COPY_AND_ASSIGN(ImmobileParticlesService);
    };
}

#endif /* Generation_PackingServices_Headers_ImmobileParticlesService_h */
