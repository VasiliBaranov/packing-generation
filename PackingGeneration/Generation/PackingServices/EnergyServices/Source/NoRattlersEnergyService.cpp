// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/NoRattlersEnergyService.h"

#include "Core/Headers/Exceptions.h"
//#include "../../../Model/Headers/Types.h"
#include "Generation/PackingServices/PostProcessing/Headers/RattlerRemovalService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/EnergyServices/Headers/IPairPotential.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/Model/Headers/Config.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    NoRattlersEnergyService::NoRattlersEnergyService(MathService* mathService, INeighborProvider* neighborProvider, RattlerRemovalService* rattlerRemovalService) :
            BaseDistanceService(mathService, neighborProvider)
    {
        potentialNormalizer = 1.0;
        minNeighborsCount = 0;
        this->rattlerRemovalService = rattlerRemovalService;
    }

    NoRattlersEnergyService::~NoRattlersEnergyService()
    {
    }

    void NoRattlersEnergyService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
        rattlerRemovalService->SetParticles(particles);
    }

    void NoRattlersEnergyService::SetContext(const ModellingContext& context)
    {
        BaseDistanceService::SetContext(context);
        rattlerRemovalService->SetContext(context);
    }

    void NoRattlersEnergyService::StartMove(ParticleIndex particleIndex)
    {
        neighborProvider->StartMove(particleIndex);
    }

    void NoRattlersEnergyService::EndMove()
    {
        neighborProvider->EndMove();
    }

    int NoRattlersEnergyService::GetMinNeighborsCount() const
    {
        return rattlerRemovalService->GetMinNeighborsCount();
    }

    void NoRattlersEnergyService::SetMinNeighborsCount(int value)
    {
        rattlerRemovalService->SetMinNeighborsCount(value);
    }

    FLOAT_TYPE NoRattlersEnergyService::GetPotentialNormalizer() const
    {
        return potentialNormalizer;
    }

    void NoRattlersEnergyService::SetPotentialNormalizer(FLOAT_TYPE value)
    {
        potentialNormalizer = value;
    }

    IEnergyService::EnergiesResult NoRattlersEnergyService::GetContractionEnergies(const vector<FLOAT_TYPE>& contractionRatios, const vector<const IPairPotential*>& pairPotentials)
    {
        const Packing& particlesRef = *particles;
        vector<bool> rattlerMask(config->particlesCount);
        int energyTypesCount = contractionRatios.size();

        IEnergyService::EnergiesResult energiesResult;
        energiesResult.contractionEnergies.resize(energyTypesCount, 0.0);
        energiesResult.nonRattlersCounts.resize(energyTypesCount, 0);

        for (int energyTypeIndex = 0; energyTypeIndex < energyTypesCount; ++energyTypeIndex)
        {
           FLOAT_TYPE contractionRatio = contractionRatios[energyTypeIndex];
           const IPairPotential* pairPotential = pairPotentials[energyTypeIndex];

           rattlerRemovalService->FillRattlerMask(contractionRatio, &rattlerMask);

           for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
           {
               const DomainParticle* particle = &particlesRef[particleIndex];
                if (rattlerMask[particleIndex])
                {
                    continue;
                }

                ParticleIndex neighborsCount;
                const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
                for (ParticleIndex i = 0; i < neighborsCount; ++i)
                {
                    ParticleIndex neighborIndex = neighborIndexes[i];
                    const Particle* neighbor = &particlesRef[neighborIndex];
                    if (rattlerMask[neighborIndex])
                    {
                        continue;
                    }

                    FLOAT_TYPE distance =  mathService->GetDistanceLength(neighbor->coordinates, particle->coordinates);
                    Nullable<FLOAT_TYPE> energy = pairPotential->GetEnergy(particle->diameter, neighbor->diameter, distance * contractionRatio);
                    if (energy.hasValue)
                    {
                        energiesResult.contractionEnergies[energyTypeIndex] += potentialNormalizer * energy.value;
                    }
                }
                energiesResult.nonRattlersCounts[energyTypeIndex]++;
            }
        }

        return energiesResult;
    }

    ParticlePair NoRattlersEnergyService::FillParticleForces(FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, vector<SpatialVector>* particleForces)
    {
        throw NotImplementedException("NoRattlersEnergyService::FillParticleForces not implemented.");
    }
}

