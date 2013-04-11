// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_IClosestPairProvider_h
#define Generation_PackingServices_DistanceServices_Headers_IClosestPairProvider_h

#include "Generation/PackingServices/Headers/IParticleService.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"

namespace PackingServices
{
    // Defines methods to efficiently determine closest particles pair.
    class IClosestPairProvider : public virtual IParticleService, public virtual IContextDependentService
    {
    public:
        virtual Model::ParticlePair FindClosestPair() const = 0;

        virtual Model::ParticlePair FindClosestNeighbor(Model::ParticleIndex particleIndex) const = 0;

        virtual ~IClosestPairProvider(){ };
    };
}


#endif /* Generation_PackingServices_DistanceServices_Headers_IClosestPairProvider_h */

