// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_DistanceService_h
#define Generation_PackingServices_DistanceServices_Headers_DistanceService_h

#include "IClosestPairProvider.h"
#include "BaseDistanceService.h"
namespace PackingServices { class GeometryService; }

namespace PackingServices
{
    // Represents a class to determine various distances in a packing, a wrapper over neighborProvider.
    class DistanceService : public BaseDistanceService, public virtual IClosestPairProvider
    {
    private:
        GeometryService* geometryService;

        struct StructureFactorPair
        {
            Core::FLOAT_TYPE waveVectorLength;
            Core::FLOAT_TYPE structureFactorValue;

            bool operator<(const StructureFactorPair& other) const
            {
                return waveVectorLength < other.waveVectorLength;
            };
        };

    public:
        DistanceService(MathService* mathService, GeometryService* geometryService, INeighborProvider* neighborProvider);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

        // Neighbors search

        Core::FLOAT_TYPE GetDistanceToNearestSurface(const Core::SpatialVector& point) const;

        Core::FLOAT_TYPE GetDistanceToNearestSurface(Model::ParticleIndex particleIndex) const;

        OVERRIDE Model::ParticlePair FindClosestPair() const;

        OVERRIDE Model::ParticlePair FindClosestNeighbor(Model::ParticleIndex particleIndex) const;

        // We could pass DomainParticle* instead of first 3 parameters, but that would be confusing: whether this particle belongs to the initial list or not or may it not belong,
        // and if the particle with the same index is excluded.
        void SetClosestNeighbor(const Core::SpatialVector& point, Model::ParticleIndex indexToExclude, Core::FLOAT_TYPE diameter, Model::ParticleIndex* neighborIndex, Core::FLOAT_TYPE* normalizedDistanceSquare) const;

        void FillPairCorrelationFunction(Model::PairCorrelationFunction* pairCorrelationFunction) const;

        void FillStructureFactor(Model::StructureFactor* structureFactor) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(DistanceService);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_DistanceService_h */
