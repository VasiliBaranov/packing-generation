// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_INeighborProvider_h
#define Generation_PackingServices_DistanceServices_Headers_INeighborProvider_h

#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IParticleService.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"

namespace PackingServices
{
    // Defines methods for finding a list of approximately neighboring particles.
    class INeighborProvider : public virtual IParticleService, public virtual IContextDependentService
    {
    public:
        // The returned lists should be internal with respect to INeighborProvider, and should not be deleted due to performance implications.
        virtual const Model::ParticleIndex* GetNeighborIndexes(Model::ParticleIndex particleIndex, Model::ParticleIndex* neighborsCount) const = 0;

        virtual const Model::ParticleIndex* GetNeighborIndexes(const Core::SpatialVector& coordinates, Model::ParticleIndex* neighborsCount) const = 0;

        // Gets the distance from the given point to the update boundary, at which the particle neighbor lists will be updated.
        // Returns a negative number if the boundary is never crossed.
        // TODO: passing particleIndex and point (particle coordinates) is very confusing (but currently necessary for VerletListNeighborProvider). Think how to resolve.
        virtual Core::FLOAT_TYPE GetTimeToUpdateBoundary(Model::ParticleIndex particleIndex, const Core::SpatialVector& point, const Core::SpatialVector& velocity) const = 0;

        virtual ~INeighborProvider(){};
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_INeighborProvider_h */

