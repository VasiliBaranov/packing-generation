// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_BondsProvider_h
#define Generation_PackingServices_DistanceServices_Headers_BondsProvider_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"

namespace PackingServices { class MathService; }
namespace PackingServices { class INeighborProvider; }

namespace PackingServices
{
    class BondsProvider
    {
    public:
        struct BondPair
        {
            int firstBondIndex;
            int secondBondIndex;

            // These fields are redundant and can be retrieved indirectly;
            // but it will be not so fast; also, bonds are never updated during generation, so no extra-updates will be needed.
            Model::ParticleIndex commonParticleIndex;
            Model::ParticleIndex firstNeighborIndex;
            Model::ParticleIndex secondNeighborIndex;
        };

        struct Statistics
        {
            Core::FLOAT_TYPE meanGapLength;
            Core::FLOAT_TYPE meanIntersectionLength;
            int gapsCount;
            int intersectionsCount;
            std::vector<Model::ParticlePair> addedBonds;
            std::vector<Model::ParticlePair> removedBonds;
        };


    private:
        // Particle indexes in all bonds should be ordered; bonds are unique (bond i-j is not duplicated with bond j-i).
        std::vector<Model::ParticlePair> bonds;

        // To store vectors below we could introduce a descendant of a DomainParticle.
        // But INeighborProvider and IClosestPairProvider accept a vector of DomainParticle, not pointers to DomainParticle,
        // and synchronization of two particle arrays will be time-consuming.
        std::vector<std::vector<int> > bondIndexesPerParticle;

        // Bond indexes in all bond pairs should be ordered; bond pairs are unique (pair i-j is not duplicated with pair j-i).
        // Each bond pair belongs to a single particle (a common particle of two bonds), so we can store bond pairs per particle, not bond pair indexes.
        std::vector<std::vector<BondPair> > bondPairsPerParticle;

        int bondPairsCount;

        Core::FLOAT_TYPE bondThreshold;

    public:
        BondsProvider();

        // This method does not follow a usual interface SetParticles/StartMove/EndMove,
        // 1. because bonds may not be updated on each particle move in ClosestJammingStep,
        // and there may be a lot of moves between new contact formation (so we don't have to update bonds on each move).
        // 2. because neighborProvider, mathService and bondsProvider are shared with ClosestJammingStep,
        // so calls to their StartMove/EndMove will be duplicated.
        // NeighborProvider and mathService are also passed here (not in the constructor) to emphasize that they should be synchronized with particles.
        Statistics UpdateBonds(const PackingServices::INeighborProvider& neighborProvider,
                const PackingServices::MathService& mathService,
                const Model::Packing& particles,
                Core::FLOAT_TYPE innerDiameterRatio,
                bool shouldRemoveBrokenBonds);

        void RemoveBonds(const std::vector<Model::ParticlePair>& bonds);

        void Reset(int particlesCount);

        bool ParticlesShareBond(Model::ParticleIndex firstParticleIndex, Model::ParticleIndex secondParticleIndex) const;

        const std::vector<Model::ParticlePair>& GetBonds() const;

        const std::vector<std::vector<int> >& GetBondIndexesPerParticle() const;

        const std::vector<std::vector<BondPair> >& GetBondPairsPerParticle() const;

        int GetBondPairsCount() const;

        Core::FLOAT_TYPE GetBondThreshold() const;

        void SetBondThreshold(Core::FLOAT_TYPE value);

        ~BondsProvider();

    private:
        bool BondExists(const Model::ParticlePair& bond) const;

        int GetBondIndex(const Model::ParticlePair& bond) const;

        void AddBond(const Model::ParticlePair& bond);

        void AddBondPairs(const Model::ParticlePair& bond, int bondIndex, bool usePairsByFirstParticle);

        void RemoveBond(int bondIndex);

        void RemoveBondInParticlesAndPairs(const Model::ParticlePair& bond, int bondIndex, bool usePairsByFirstParticle);

        void ChangeBondIndexInParticlesAndPairs(const Model::ParticlePair& movedBond, int oldMovedBondIndex, int newMovedBondIndex, bool useFirstParticle);

        Model::ParticlePair CreateBond(Model::ParticleIndex firstParticleIndex, Model::ParticleIndex secondParticleIndex) const;

        BondPair CreateBondPair(int firstBondIndex, int secondBondIndex) const;

        DISALLOW_COPY_AND_ASSIGN(BondsProvider);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_BondsProvider_h */
