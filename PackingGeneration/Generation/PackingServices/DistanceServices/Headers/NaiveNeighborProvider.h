// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_NaiveNeighborProvider_h
#define Generation_PackingServices_DistanceServices_Headers_NaiveNeighborProvider_h

#include "INeighborProvider.h"
#include "Generation/Model/Headers/Types.h"
#include "Core/Headers/Macros.h"
namespace PackingServices { class GeometryCollisionService; }
namespace Model { class SystemConfig; }

namespace PackingServices
{
    // A naive implementation of a neighbor provider: returns all the packing particles as neighbours of a current particle.
    class NaiveNeighborProvider : public virtual INeighborProvider
    {
    private:
        GeometryCollisionService* geometryCollisionService;
        const Model::SystemConfig* config;
        Model::CubicBox box;
        // TODO: think how to remove mutable
        mutable std::vector<Model::ParticleIndex> allNeighborIndexes;
        mutable Model::ParticleIndex lastUsedParticleIndex;

    public:
        explicit NaiveNeighborProvider(GeometryCollisionService* geometryCollisionService);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetParticles(const Model::DomainParticle& particles);

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(Model::ParticleIndex particleIndex, Model::ParticleIndex* neighborsCount) const;

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(const Core::SpatialVector& coordinates, Model::ParticleIndex* neighborsCount) const;

        OVERRIDE Core::FLOAT_TYPE GetTimeToUpdateBoundary(Model::ParticleIndex particleIndex, const Core::SpatialVector& point, const Core::SpatialVector& velocity) const;

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

        virtual ~NaiveNeighborProvider();

    private:
        DISALLOW_COPY_AND_ASSIGN(NaiveNeighborProvider);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_NaiveNeighborProvider_h */

