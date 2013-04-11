// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_NoRattlersEnergyService_h
#define Generation_PackingServices_EnergyServices_Headers_NoRattlersEnergyService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"
#include "IEnergyService.h"
namespace PackingServices { class RattlerRemovalService; }

namespace PackingServices
{
    class NoRattlersEnergyService : public BaseDistanceService, public virtual IEnergyService
    {
    public:
        Core::FLOAT_TYPE potentialNormalizer;
        int minNeighborsCount;
        RattlerRemovalService* rattlerRemovalService;

    public:
        NoRattlersEnergyService(MathService* mathService, INeighborProvider* neighborProvider, RattlerRemovalService* rattlerRemovalService);

        ~NoRattlersEnergyService();

        OVERRIDE Core::FLOAT_TYPE GetPotentialNormalizer() const;

        OVERRIDE void SetPotentialNormalizer(Core::FLOAT_TYPE value);

        OVERRIDE int GetMinNeighborsCount() const;

        OVERRIDE void SetMinNeighborsCount(int value);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

        // Elastic energy functions

        OVERRIDE EnergiesResult GetContractionEnergies(const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<const IPairPotential*>& pairPotentials);

        OVERRIDE Model::ParticlePair FillParticleForces(Core::FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, std::vector<Core::SpatialVector>* particleForces);

    private:
        DISALLOW_COPY_AND_ASSIGN(NoRattlersEnergyService);
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_NoRattlersEnergyService_h */

