// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_EnergyService_h
#define Generation_PackingServices_EnergyServices_Headers_EnergyService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"
#include "IEnergyService.h"

namespace PackingServices
{
    class EnergyService : public BaseDistanceService, public virtual IEnergyService
    {
    private:
        std::vector<Core::SpatialVector> vectorsToNeighbors;
        std::vector<Core::FLOAT_TYPE> distancesToNeighbors;
        const Model::ParticleIndex* neighborIndexes;
        Model::ParticleIndex neighborsCount;

        // Close neighbors search
        std::vector<bool> closeNeighborsMask;
        std::vector<Model::ParticleIndex> sortingPermutation;
        std::vector<Core::FLOAT_TYPE> distancesToNeighborSurfaces;

        Core::FLOAT_TYPE potentialNormalizer;
        int minNeighborsCount;

    public:
        // Max number of close neighbors to contribute to particle energy and force
        int maxCloseNeighborsCount;

    public:
        EnergyService(MathService* mathService, INeighborProvider* neighborProvider);

        OVERRIDE Core::FLOAT_TYPE GetPotentialNormalizer() const;

        OVERRIDE void SetPotentialNormalizer(Core::FLOAT_TYPE value);

        OVERRIDE int GetMinNeighborsCount() const;

        OVERRIDE void SetMinNeighborsCount(int value);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

        // Elastic energy functions

        OVERRIDE EnergiesResult GetContractionEnergies(const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<const IPairPotential*>& pairPotentials);

        OVERRIDE Model::ParticlePair FillParticleForces(Core::FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, std::vector<Core::SpatialVector>* particleForces);

    private:
        void FilterCloseNeighbors();

        void FillDistancesToNeighbors(Model::ParticleIndex particleIndex);

        void FillDistancesAndDirectionsToNeighbors(Model::ParticleIndex particleIndex);

        void UpdateClosestPair(Model::ParticleIndex particleIndex, Model::ParticlePair* closestPair);

        void FillCurrentParticleEnergies(Model::ParticleIndex particleIndex,
                const std::vector<Core::FLOAT_TYPE>& contractionRatios,
                const std::vector<const IPairPotential*>& pairPotentials,
                std::vector<Core::FLOAT_TYPE>* currentParticleEnergies,
                std::vector<int>* currentParticleNeighborsCounts);

        void FillCurrentParticleForce(Model::ParticleIndex particleIndex, Core::FLOAT_TYPE contractionRatio, const IPairPotential& pairPotential, Core::SpatialVector* force);

        DISALLOW_COPY_AND_ASSIGN(EnergyService);

    private:
        template<class T>
        class IndexesComparer
        {
        private:
            const std::vector<T>& values;

        public:
            IndexesComparer(const std::vector<T>& valuesArray) : values(valuesArray)
            {
            };

            bool operator()(int i, int j)
            {
                return values[i] < values[j];
            };
        };
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_EnergyService_h */

