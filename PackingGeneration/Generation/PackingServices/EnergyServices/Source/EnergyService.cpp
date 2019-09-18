// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/EnergyService.h"

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Constants.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "../Headers/IPairPotential.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    EnergyService::EnergyService(MathService* mathService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {
        minNeighborsCount = 0;
        maxCloseNeighborsCount = -1;
    }

    void EnergyService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    void EnergyService::StartMove(ParticleIndex particleIndex)
    {
        neighborProvider->StartMove(particleIndex);
    }

    void EnergyService::EndMove()
    {
        neighborProvider->EndMove();
    }

    IEnergyService::EnergiesResult EnergyService::GetContractionEnergies(const vector<FLOAT_TYPE>& contractionRatios, const vector<const IPairPotential*>& pairPotentials)
    {
        int energyTypesCount = contractionRatios.size();
        IEnergyService::EnergiesResult energiesResult;

        energiesResult.contractionEnergies.resize(energyTypesCount, 0.0);
        energiesResult.nonRattlersCounts.resize(energyTypesCount, 0);
        energiesResult.closestPair.normalizedDistanceSquare = MAX_FLOAT_VALUE;

        vector<int> currentParticleNeighborsCounts(energyTypesCount);
        vector<FLOAT_TYPE> currentParticleEnergies(energyTypesCount);
        vector<vector<int>> currentParticleNeighborIndexes(energyTypesCount);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            FillDistancesToNeighbors(particleIndex);

            UpdateClosestPair(particleIndex, &energiesResult.closestPair);

            FilterCloseNeighbors();

            FillCurrentParticleEnergies(particleIndex, 
                contractionRatios, 
                pairPotentials, 
                &currentParticleEnergies, 
                &currentParticleNeighborsCounts,
                &currentParticleNeighborIndexes);

            for (int energyTypeIndex = 0; energyTypeIndex < energyTypesCount; ++energyTypeIndex)
            {
                // 4 is the isostatic number for infinite friction and also a min number of contacts for particle mechanical stability.
                // This is a quick fix, as if we should remove particles with less than 4 contacts recursively (as some of them may contribute to the contacts of other particles).
                if (currentParticleNeighborsCounts[energyTypeIndex] >= minNeighborsCount)
                {
                    energiesResult.contractionEnergies[energyTypeIndex] += currentParticleEnergies[energyTypeIndex];
                    energiesResult.nonRattlersCounts[energyTypeIndex]++;
                }
            }
        }

        return energiesResult;
    }

    // TODO: Merge with GetContractionEnergies!!!!
    OVERRIDE void EnergyService::GetContractionEnergiesPerParticle(const vector<FLOAT_TYPE>& contractionRatios, 
        const vector<const IPairPotential*>& pairPotentials, 
        vector<IEnergyService::EnergiesPerParticle>* energiesPerParticle)
    {
        size_t energyTypesCount = contractionRatios.size();
        vector<IEnergyService::EnergiesPerParticle>& energiesPerParticleRef = *energiesPerParticle;
        energiesPerParticleRef.resize(energyTypesCount);
        for (size_t energyTypeIndex = 0; energyTypeIndex < energyTypesCount; ++energyTypeIndex)
        {
            energiesPerParticleRef[energyTypeIndex].contractionEnergiesPerParticle.resize(config->particlesCount);
            energiesPerParticleRef[energyTypeIndex].rattlerMask.resize(config->particlesCount);
            energiesPerParticleRef[energyTypeIndex].touchingNeighborIndexesPerParticle.resize(config->particlesCount);
        }

        vector<int> currentParticleNeighborsCounts(energyTypesCount);
        vector<FLOAT_TYPE> currentParticleEnergies(energyTypesCount);
        vector<vector<int>> currentParticleTouchingNeighborsIndexes(energyTypesCount);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            FillDistancesToNeighbors(particleIndex);

            FilterCloseNeighbors();

            currentParticleTouchingNeighborsIndexes.clear();

            FillCurrentParticleEnergies(particleIndex, 
                contractionRatios, 
                pairPotentials, 
                &currentParticleEnergies, 
                &currentParticleNeighborsCounts,
                &currentParticleTouchingNeighborsIndexes);

            for (size_t energyTypeIndex = 0; energyTypeIndex < energyTypesCount; ++energyTypeIndex)
            {
                energiesPerParticleRef[energyTypeIndex].contractionEnergiesPerParticle[particleIndex] = currentParticleEnergies[energyTypeIndex];
                energiesPerParticleRef[energyTypeIndex].rattlerMask[particleIndex] = currentParticleNeighborsCounts[energyTypeIndex] < minNeighborsCount;
                energiesPerParticleRef[energyTypeIndex].touchingNeighborIndexesPerParticle[particleIndex].swap(currentParticleTouchingNeighborsIndexes[energyTypeIndex]);
                //energiesPerParticleRef[energyTypeIndex].touchingNeighborIndexesPerParticle[particleIndex] = currentParticleTouchingNeighborsIndexes[energyTypeIndex];
            }
        }
    }

    ParticlePair EnergyService::FillParticleForces(FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, vector<SpatialVector>* particleForces)
    {
        ParticlePair closestPair;
        closestPair.normalizedDistanceSquare = MAX_FLOAT_VALUE;
        vector<SpatialVector>& particleForcesRef = *particleForces;

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            FillDistancesAndDirectionsToNeighbors(particleIndex);

            UpdateClosestPair(particleIndex, &closestPair);

            FilterCloseNeighbors();

            FillCurrentParticleForce(particleIndex, contractionRatio, pairPotential, &particleForcesRef[particleIndex]);
        }

        return closestPair;
    }

    int EnergyService::GetMinNeighborsCount() const
    {
        return minNeighborsCount;
    }

    void EnergyService::SetMinNeighborsCount(int value)
    {
        minNeighborsCount = value;
    }

    FLOAT_TYPE EnergyService::GetPotentialNormalizer() const
    {
        return potentialNormalizer;
    }

    void EnergyService::SetPotentialNormalizer(Core::FLOAT_TYPE value)
    {
        potentialNormalizer = value;
    }

    void EnergyService::FilterCloseNeighbors()
    {
        if (maxCloseNeighborsCount <= 0)
        {
            return;
        }

        const Packing& particlesRef = *particles;
        closeNeighborsMask.resize(neighborsCount);
        VectorUtilities::InitializeWith(&closeNeighborsMask, false);

        distancesToNeighborSurfaces.resize(neighborsCount);
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle* neighbor = &particlesRef[neighborIndex];

            //No need to subtract current particle radius, as it's equal for all the entries, and won't affect sorting results
            distancesToNeighborSurfaces[i] = distancesToNeighbors[i] - neighbor->diameter * 0.5;
        }

        // NOTE: Here we compare distances to neighbor sphere surfaces, not normalized distances,
        // as we would like to treat small and large particles equally while energy minimization (to ensure isostaticity as early as possible).
        // TODO: this is algorithm-specific, so we may extract it later into a separate IParticleEnergyService or add IPotentialCutoffService.
        StlUtilities::FindNthElementPermutation(distancesToNeighborSurfaces, maxCloseNeighborsCount - 1, &sortingPermutation);

        for (ParticleIndex i = 0; i < maxCloseNeighborsCount; ++i)
        {
            ParticleIndex closeNeighborIndex = sortingPermutation[i];
            closeNeighborsMask[closeNeighborIndex] = true;
        }
    }

    void EnergyService::FillCurrentParticleForce(ParticleIndex particleIndex, FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, SpatialVector* force)
    {
        const Packing& particlesRef = *particles;
        const DomainParticle& particle = particlesRef[particleIndex];
        VectorUtilities::InitializeWith(force, 0.0);

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            if (maxCloseNeighborsCount > 0 && !closeNeighborsMask[i])
            {
                continue;
            }

            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle& neighbor = particlesRef[neighborIndex];

            FLOAT_TYPE distance = distancesToNeighbors[i];
            SpatialVector difference = vectorsToNeighbors[i];

            Nullable<FLOAT_TYPE> forceLength = pairPotential.GetRepulsionForceFromNeighbor(particle.diameter, neighbor.diameter, distance * contractionRatio);
            if (!forceLength.hasValue)
            {
                continue;
            }

            SpatialVector neighborForce;
            // Difference is directed from the particle to the neighbor, so to get the force we need to multiply it by -1.
            VectorUtilities::MultiplyByValue(difference, - forceLength.value / distance, &neighborForce);
            VectorUtilities::Add(*force, neighborForce, force);
        }
    }

    void EnergyService::FillDistancesToNeighbors(ParticleIndex particleIndex)
    {
        const Packing& particlesRef = *particles;
        const DomainParticle* particle = &particlesRef[particleIndex];

        neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
        distancesToNeighbors.resize(neighborsCount);
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle* neighbor = &particlesRef[neighborIndex];

            distancesToNeighbors[i] = mathService->GetDistanceLength(neighbor->coordinates, particle->coordinates);
        }
    }

    void EnergyService::FillDistancesAndDirectionsToNeighbors(ParticleIndex particleIndex)
    {
        const Packing& particlesRef = *particles;
        const DomainParticle& particle = particlesRef[particleIndex];

        neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
        distancesToNeighbors.resize(neighborsCount);
        vectorsToNeighbors.resize(neighborsCount);
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle& neighbor = particlesRef[neighborIndex];

            mathService->FillDistance(neighbor.coordinates, particle.coordinates, &vectorsToNeighbors[i]);
            distancesToNeighbors[i] = VectorUtilities::GetLength(vectorsToNeighbors[i]);
        }
    }

    void EnergyService::FillCurrentParticleEnergies(ParticleIndex particleIndex,
            const vector<FLOAT_TYPE>& contractionRatios,
            const vector<const IPairPotential*>& pairPotentials,
            vector<FLOAT_TYPE>* currentParticleEnergies,
            vector<int>* currentParticleNeighborsCounts,
            vector<vector<int>>* currentParticleTouchingNeighborsIndexes)
    {
        int energyTypesCount = contractionRatios.size();
        vector<int>& currentParticleNeighborsCountsRef = *currentParticleNeighborsCounts;
        vector<FLOAT_TYPE>& currentParticleEnergiesRef = *currentParticleEnergies;
        vector<vector<int>>& currentParticleTouchingNeighborsIndexesRef = *currentParticleTouchingNeighborsIndexes;
        const Packing& particlesRef = *particles;
        currentParticleNeighborsCountsRef.resize(energyTypesCount, 0);
        currentParticleEnergiesRef.resize(energyTypesCount, 0.0);

        currentParticleTouchingNeighborsIndexesRef.clear();
        currentParticleTouchingNeighborsIndexesRef.resize(energyTypesCount);

        const DomainParticle& particle = particlesRef[particleIndex];

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            if (maxCloseNeighborsCount > 0 && !closeNeighborsMask[i])
            {
                continue;
            }

            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle& neighbor = particlesRef[neighborIndex];

            FLOAT_TYPE distance = distancesToNeighbors[i];

            for (int energyTypeIndex = 0; energyTypeIndex < energyTypesCount; ++energyTypeIndex)
            {
                FLOAT_TYPE contractionRatio = contractionRatios[energyTypeIndex];
                const IPairPotential* pairPotential = pairPotentials[energyTypeIndex];

                Nullable<FLOAT_TYPE> energy = pairPotential->GetEnergy(particle.diameter, neighbor.diameter, distance * contractionRatio);
                if (energy.hasValue)
                {
                    currentParticleEnergiesRef[energyTypeIndex] += potentialNormalizer * energy.value;
                    currentParticleNeighborsCountsRef[energyTypeIndex]++;
                    currentParticleTouchingNeighborsIndexesRef[energyTypeIndex].push_back(neighborIndex);
                }
            }
        }
    }

    void EnergyService::UpdateClosestPair(ParticleIndex particleIndex, ParticlePair* closestPair)
    {
        const Packing& particlesRef = *particles;
        const DomainParticle* particle = &particlesRef[particleIndex];

        for (int i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            const Particle* neighbor = &particlesRef[neighborIndex];

            FLOAT_TYPE distance = distancesToNeighbors[i];
            FLOAT_TYPE diameterRatio = 2.0 * distance / (particle->diameter + neighbor->diameter);

            if (closestPair->normalizedDistanceSquare > diameterRatio * diameterRatio)
            {
                closestPair->normalizedDistanceSquare = diameterRatio * diameterRatio;
                closestPair->firstParticleIndex = particleIndex;
                closestPair->secondParticleIndex = neighborIndex;
            }
        }
    }
}

