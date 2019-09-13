// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_IEnergyService_h
#define Generation_PackingServices_EnergyServices_Headers_IEnergyService_h

#include "Generation/PackingServices/Headers/IParticleService.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace PackingServices { class IPairPotential; }

namespace PackingServices
{
    class IEnergyService : public virtual IParticleService, public virtual IContextDependentService
    {
    public:
        struct EnergiesResult
        {
            Model::ParticlePair closestPair;
            std::vector<Core::FLOAT_TYPE> contractionEnergies;
            std::vector<int> nonRattlersCounts;
        };

        struct EnergiesPerParticle
        {
            std::vector<Core::FLOAT_TYPE> contractionEnergiesPerParticle;
            std::vector<bool> rattlerMask;
            std::vector<std::vector<int>> touchingNeighborIndexesPerParticle;
        };

    public:
        virtual Core::FLOAT_TYPE GetPotentialNormalizer() const = 0;

        virtual void SetPotentialNormalizer(Core::FLOAT_TYPE value) = 0;

        // I would prefer to make IPairPotential an internal field of EnergyService, but passing several potentials here allows calculation energies 2 times faster (about 1 minute instead of 2 per packing, 300 packings)
        virtual EnergiesResult GetContractionEnergies(const std::vector<Core::FLOAT_TYPE>& contractionRatios, 
            const std::vector<const IPairPotential*>& pairPotentials) = 0;

        // TODO: sync this methods with GetContractionEnergies (should return rattlers or not)
        virtual Model::ParticlePair FillParticleForces(Core::FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, std::vector<Core::SpatialVector>* particleForces) = 0;

        virtual void GetContractionEnergiesPerParticle(const std::vector<Core::FLOAT_TYPE>& contractionRatios, 
            const std::vector<const IPairPotential*>& pairPotentials, 
            std::vector<EnergiesPerParticle>* energiesPerParticle) = 0;

        // Gets a min number of neighbors used to consider a particle as a non-rattler (inclusive)
        virtual int GetMinNeighborsCount() const = 0;

        // Sets a min number of neighbors used to consider a particle as a non-rattler (inclusive)
        virtual void SetMinNeighborsCount(int value) = 0;

        virtual ~IEnergyService(){ };
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_IEnergyService_h */

