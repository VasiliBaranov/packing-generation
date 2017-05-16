// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include <cmath>
#include "../Headers/BondsProvider.h"
#include "Core/Headers/StlUtilities.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    BondsProvider::BondsProvider()
    {
        bondThreshold = 1e-10;
    }

    BondsProvider::~BondsProvider()
    {
    }

    const vector<ParticlePair>& BondsProvider::GetBonds() const
    {
        return bonds;
    }

    const vector<vector<int> >& BondsProvider::GetBondIndexesPerParticle() const
    {
        return bondIndexesPerParticle;
    }

    const vector<vector<BondsProvider::BondPair> >& BondsProvider::GetBondPairsPerParticle() const
    {
        return bondPairsPerParticle;
    }

    int BondsProvider::GetBondPairsCount() const
    {
        return bondPairsCount;
    }

    FLOAT_TYPE BondsProvider::GetBondThreshold() const
    {
        return bondThreshold;
    }

    void BondsProvider::SetBondThreshold(Core::FLOAT_TYPE value)
    {
        bondThreshold = value;
    }

    void BondsProvider::Reset(int particlesCount)
    {
        bonds.clear();

        bondPairsPerParticle.clear();
        bondPairsPerParticle.resize(particlesCount);
        bondPairsCount = 0;

        bondIndexesPerParticle.clear();
        bondIndexesPerParticle.resize(particlesCount);
    }

    BondsProvider::Statistics BondsProvider::UpdateBonds(const INeighborProvider& neighborProvider,
            const MathService& mathService,
            const Packing& particles,
            FLOAT_TYPE innerDiameterRatio,
            bool shouldRemoveBrokenBonds)
    {
        BondsProvider::Statistics statistics;
        statistics.meanGapLength = 0;
        statistics.meanIntersectionLength = 0;
        statistics.gapsCount = 0;
        statistics.intersectionsCount = 0;

        for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex)
        {
            const DomainParticle& particle = particles[particleIndex];

            int neighborsCount;
            const int* neighborIndexes = neighborProvider.GetNeighborIndexes(particleIndex, &neighborsCount);

            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                const DomainParticle& neighbor = particles[neighborIndex];

                FLOAT_TYPE normalizedDistance = mathService.GetNormalizedDistance(particle, neighbor);
                const FLOAT_TYPE tolerance = 1.0 + bondThreshold;
                if (normalizedDistance < innerDiameterRatio * tolerance)
                {
                    // Add bond
                    ParticlePair bond = CreateBond(particleIndex, neighborIndex);
                    if (!BondExists(bond))
                    {
                        statistics.addedBonds.push_back(bond);
                        AddBond(bond);
                    }
                    if (normalizedDistance < innerDiameterRatio)
                    {
                        statistics.intersectionsCount++;
                        statistics.meanIntersectionLength += (innerDiameterRatio - normalizedDistance);
                    }
                }
                else
                {
                    ParticlePair bond = CreateBond(particleIndex, neighborIndex);
                    int bondIndex = GetBondIndex(bond);
                    if (bondIndex != -1)
                    {
                        statistics.meanGapLength += (normalizedDistance - innerDiameterRatio);
                        statistics.gapsCount++;

                        if (shouldRemoveBrokenBonds)
                        {
                            statistics.removedBonds.push_back(bond);
                            RemoveBond(bondIndex);
                        }
                    }
                }
            }
        }

        if (statistics.gapsCount > 0)
        {
            statistics.meanGapLength /= statistics.gapsCount;
            statistics.gapsCount /= 2;
        }
        if (statistics.intersectionsCount > 0)
        {
            statistics.meanIntersectionLength /= statistics.intersectionsCount;
            statistics.intersectionsCount /= 2;
        }
        return statistics;
    }

    void BondsProvider::RemoveBonds(const vector<ParticlePair>& bonds)
    {
        for (size_t i = 0; i < bonds.size(); ++i)
        {
            const ParticlePair& bond = bonds[i];
            int bondIndex = GetBondIndex(bond);
            if (bondIndex != -1)
            {
                RemoveBond(bondIndex);
            }
        }
    }

    void BondsProvider::AddBond(const ParticlePair& bond)
    {
        int bondIndex = bonds.size();
        bonds.push_back(bond);

        // Add bond pairs
        AddBondPairs(bond, bondIndex, true);
        AddBondPairs(bond, bondIndex, false);

        bondIndexesPerParticle[bond.firstParticleIndex].push_back(bondIndex);
        bondIndexesPerParticle[bond.secondParticleIndex].push_back(bondIndex);
    }

    void BondsProvider::AddBondPairs(const ParticlePair& bond, int bondIndex, bool usePairsByFirstParticle)
    {
        ParticleIndex commonParticleIndex = usePairsByFirstParticle ? bond.firstParticleIndex : bond.secondParticleIndex;
        ParticleIndex otherParticleIndex = usePairsByFirstParticle ? bond.secondParticleIndex : bond.firstParticleIndex;

        const vector<int>& commonParticleBondIndexes = bondIndexesPerParticle[commonParticleIndex];
        for (size_t i = 0; i < commonParticleBondIndexes.size(); ++i)
        {
            int commonParticleBondIndex = commonParticleBondIndexes[i];
            const ParticlePair& otherBond = bonds[commonParticleBondIndex];

            BondPair newPair = CreateBondPair(bondIndex, commonParticleBondIndex);
            newPair.commonParticleIndex = commonParticleIndex;
            newPair.firstNeighborIndex = otherParticleIndex;
            newPair.secondNeighborIndex = otherBond.GetOtherIndex(commonParticleIndex);

            bondPairsPerParticle[commonParticleIndex].push_back(newPair);
            bondPairsCount++;
        }
    }

    void BondsProvider::RemoveBond(int bondIndex)
    {
        const ParticlePair& bond = bonds[bondIndex];
        RemoveBondInParticlesAndPairs(bond, bondIndex, true);
        RemoveBondInParticlesAndPairs(bond, bondIndex, false);

        StlUtilities::QuicklyRemoveAt(&bonds, bondIndex);

        // The last bond was moved to the bond index.
        int oldMovedBondIndex = bonds.size();

        // It was the only bond at all or it was the very last bond in the list (so that it was simply deleted)
        if (oldMovedBondIndex == 0 || oldMovedBondIndex == bondIndex)
        {
            return;
        }

        const ParticlePair& movedBond = bonds[bondIndex];

        // We shall update moved bond index for the first and second particle,
        // and should update moved bond index in each particle bond pairs.
        ChangeBondIndexInParticlesAndPairs(movedBond, oldMovedBondIndex, bondIndex, true);
        ChangeBondIndexInParticlesAndPairs(movedBond, oldMovedBondIndex, bondIndex, false);
    }

    void BondsProvider::ChangeBondIndexInParticlesAndPairs(const ParticlePair& movedBond, int oldMovedBondIndex, int newMovedBondIndex, bool useFirstParticle)
    {
        ParticleIndex particleIndex = useFirstParticle ? movedBond.firstParticleIndex : movedBond.secondParticleIndex;

        StlUtilities::Replace(&bondIndexesPerParticle[particleIndex], oldMovedBondIndex, newMovedBondIndex);

        // And also shall update bond pairs
        vector<BondPair>& bondPairs = bondPairsPerParticle[particleIndex];
        for (size_t bondPairIndex = 0; bondPairIndex < bondPairs.size(); ++bondPairIndex)
        {
            BondPair& bondPair = bondPairs[bondPairIndex];
            if (bondPair.firstBondIndex == oldMovedBondIndex)
            {
                bondPair.firstBondIndex = newMovedBondIndex;
            }
            else if (bondPair.secondBondIndex == oldMovedBondIndex)
            {
                bondPair.secondBondIndex = newMovedBondIndex;
            }

            if (bondPair.firstBondIndex > bondPair.secondBondIndex)
            {
                swap(bondPair.firstBondIndex, bondPair.secondBondIndex);
            }
        }
    }

    void BondsProvider::RemoveBondInParticlesAndPairs(const Model::ParticlePair& bond, int bondIndex, bool useFirstParticle)
    {
        ParticleIndex particleIndex = useFirstParticle ? bond.firstParticleIndex : bond.secondParticleIndex;

        StlUtilities::QuicklyRemove(&bondIndexesPerParticle[particleIndex], bondIndex);

        vector<BondPair>& bondPairs = bondPairsPerParticle[particleIndex];
        for (size_t i = 0; i < bondPairs.size(); ++i)
        {
            BondPair& bondPair = bondPairs[i];
            if (bondPair.firstBondIndex == bondIndex || bondPair.secondBondIndex == bondIndex)
            {
                StlUtilities::QuicklyRemoveAt(&bondPairs, i);
                bondPairsCount--;
                i--; // at the end of the cycle i will be increased, and we will recheck the element that appeared on the i-th position.
            }
        }
    }

    BondsProvider::BondPair BondsProvider::CreateBondPair(int firstBondIndex, int secondBondIndex) const
    {
        bool firstIndexIsLower = firstBondIndex < secondBondIndex;
        ParticleIndex minIndex = firstIndexIsLower ? firstBondIndex : secondBondIndex;
        ParticleIndex maxIndex = firstIndexIsLower ? secondBondIndex : firstBondIndex;

        BondPair newPair;
        newPair.firstBondIndex = minIndex;
        newPair.secondBondIndex = maxIndex;
        return newPair;
    }

    bool BondsProvider::ParticlesShareBond(ParticleIndex firstParticleIndex, ParticleIndex secondParticleIndex) const
    {
        ParticlePair bond = CreateBond(firstParticleIndex, secondParticleIndex);
        return BondExists(bond);
    }

    bool BondsProvider::BondExists(const ParticlePair& bond) const
    {
        int bondIndex = GetBondIndex(bond);
        return bondIndex != -1;
    }

    int BondsProvider::GetBondIndex(const Model::ParticlePair& bond) const
    {
        const vector<int>& existingBondIndexes = bondIndexesPerParticle[bond.firstParticleIndex];

        // There will be no more than 12 bonds per particle (at the end of the generation there will 6 bonds per particle on average).
        // So this direct search will be very fast, and i do not use any advanced storage for bonds per particle, e.g. hashmap or tree.
        for (size_t i = 0; i < existingBondIndexes.size(); ++i)
        {
            int bondIndex = existingBondIndexes[i];
            const ParticlePair& existingBond = bonds[bondIndex];

            // Particle indexes are ordered, so can compare in predefined order
            if (bond.firstParticleIndex == existingBond.firstParticleIndex &&
                    bond.secondParticleIndex == existingBond.secondParticleIndex)
            {
                return bondIndex;
            }
        }

        return -1;
    }

    ParticlePair BondsProvider::CreateBond(ParticleIndex firstParticleIndex, ParticleIndex secondParticleIndex) const
    {
        bool firstIndexIsLower = firstParticleIndex < secondParticleIndex;
        ParticleIndex minIndex = firstIndexIsLower ? firstParticleIndex : secondParticleIndex;
        ParticleIndex maxIndex = firstIndexIsLower ? secondParticleIndex : firstParticleIndex;

        return ParticlePair(minIndex, maxIndex, 0.0);
    }
}

