// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_ClosestPairProvider_h
#define Generation_PackingServices_DistanceServices_Headers_ClosestPairProvider_h

#include "BaseDistanceService.h"
#include "IClosestPairProvider.h"
#include "Core/Headers/OrderedPriorityQueue.h"

namespace PackingServices
{
    // Represents a class to efficiently determine closest particles pair.
    // TODO: use Lubachevsky-Stillinger event updates scheme (use closest distances instead of collision times), it will make operation several times faster. See CollisionEventProvider.
    class ClosestPairProvider : public BaseDistanceService, public virtual IClosestPairProvider
    {
    private:
        // Represents a single particle in the domain
        struct ParticleWithNeighbor : Model::DomainParticle
        {
            Model::ParticleIndex closestNeighborIndex;
            Core::FLOAT_TYPE closestNormalizedDistanceSquare;
        };

        class ParticleWithNeighborComparer
        {
        public:
            bool operator()(const ParticleWithNeighbor& first, const ParticleWithNeighbor& second)
            {
                // Invalid events (with event time < global time) will never appear in the events queue, so we do not check global time.
                return first.closestNormalizedDistanceSquare < second.closestNormalizedDistanceSquare;
            };
        };

        Model::ParticleIndex movedParticleIndex;

        std::vector<ParticleWithNeighbor> particlesWithNeighbors;

        Core::OrderedPriorityQueue<std::vector<ParticleWithNeighbor>, ParticleWithNeighborComparer> neighborDistancesQueue;

    public:
        ClosestPairProvider(MathService* mathService, INeighborProvider* neighborProvider);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        OVERRIDE Model::ParticlePair FindClosestPair() const;

        OVERRIDE Model::ParticlePair FindClosestNeighbor(Model::ParticleIndex particleIndex) const;

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

        virtual ~ClosestPairProvider();

    private:
        void FillClosestNeighbor(ParticleWithNeighbor* particle, Model::ParticleIndex indexToExclude);

        void AddParticleToNeighbors(ParticleWithNeighbor* particle);

        void RemoveParticleFromNeighbors(ParticleWithNeighbor* particle);

        static bool CompareParticles(ParticleWithNeighbor* first, ParticleWithNeighbor* second);

        DISALLOW_COPY_AND_ASSIGN(ClosestPairProvider);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_ClosestPairProvider_h */

