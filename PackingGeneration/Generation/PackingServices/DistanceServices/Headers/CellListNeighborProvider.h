// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_CellListNeighborProvider_h
#define Generation_PackingServices_DistanceServices_Headers_CellListNeighborProvider_h

#include <map>
#include "Core/Geometry/Headers/GeometryParameters.h"
#include "Core/Lattice/Headers/LatticeIndexingProvider.h"
#include "Core/Lattice/Headers/GenericLattice.h"
#include "Core/Lattice/Headers/ColumnMajorIndexingProvider.h"
#include "Generation/Model/Headers/Types.h"
#include "INeighborProvider.h"
namespace PackingServices { class GeometryCollisionService; }
namespace PackingServices { class GeometryService; }
namespace Model { class SystemConfig; }

namespace PackingServices
{
    // Represents a class to efficiently determine nearest neighbors of a current particle.
    // Uses Cell Lists algorithm: see http://en.wikipedia.org/wiki/Cell_lists and Raschdorf, Kolonko (2011) A comparison of data structures for the simulation of polydisperse particle packings.
    // Better data structures are octtree, loose octtree. But when using Verlet lists as a decorator of this neighbor provider, the alternatives are not crucial.
    class CellListNeighborProvider : public virtual INeighborProvider
    {
    private:
        // Represents a single cell in the spatial domain, tracking particles belonging to this cell.
        // In a standard implementation, a cubic domain is devided by, e.g. 4x4x4 = 64 cells, each particle belongs to one cell.
        // When searching for the close neighbors, particle lists from 27 neighboring cells are joined.
        // In this implementation, each cell contains particles from 27 neighboring cells from the previous implementation, each particle belongs to 27 cells.
        // It allows to avoid joining of 27 lists at every neighbor search, but makes updating cells more costly.
        // But even for moderately dense packings particles cross cell boundaries very rarely, so this overhead is negligible.
        struct Cell : public Model::CubicBox
        {
            std::vector<Model::ParticleIndex> particleIndexes;
            std::vector<int> neighborCellIndexes; // for each cell index stores all its neighbors
            std::map<Model::ParticleIndex, int> particleIndexesPermutation; // particleIndexesPermutation[i] gets the index of the particle i in the particleIndexes array

            Cell()
            {
                particleIndexes.reserve(50);
                neighborCellIndexes.reserve(50);
            }
        };

        // Services
        // Declare as pointers and pass to the constructor if you need to make them replaceable (primarily for tests).
        Core::GeometryParameters geometryParameters;
        Core::ColumnMajorIndexingProvider linearIndexingProvider;
        Core::GenericLattice lattice;
        Core::LatticeIndexingProvider latticeIndexingProvider;

        GeometryService* geometryService;
        GeometryCollisionService* geometryCollisionService;

        // Main variables
        const Model::Packing* particles;
        const Model::SystemConfig* config;

        // Cached variables
        mutable std::vector<Cell> domainCells; // see GetNeighborIndexes for the need for mutable
        Core::DiscreteSpatialVector cellsCounts;
        Core::SpatialVector cellSize;

        Model::ParticleIndex movedParticleIndex;
        Core::DiscreteSpatialVector previousLatticePoint;

        static const int MAX_NEIGHBORS_COUNT = 200;
        static const int NOT_PRESENT_PERMUTATION = -1;

    public:
        CellListNeighborProvider(GeometryService* geometryService, GeometryCollisionService* geometryCollisionService);

        virtual ~CellListNeighborProvider();

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(Model::ParticleIndex particleIndex, Model::ParticleIndex* neighborsCount) const;

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(const Core::SpatialVector& coordinates, Model::ParticleIndex* neighborsCount) const;

        OVERRIDE Core::FLOAT_TYPE GetTimeToUpdateBoundary(Model::ParticleIndex particleIndex, const Core::SpatialVector& point, const Core::SpatialVector& velocity) const;

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

    private:

        Model::ParticleIndex* GetNeighborIndexes(const Core::SpatialVector& coordinates, Model::ParticleIndex* neighborsCount, int* linearCellIndex) const;

        void FillDomainCellCoordinates(const Core::SpatialVector& point, Core::DiscreteSpatialVector* latticePoint) const;

        void InitializeCellDimensions();

        void InitializeDomainCells();

        void SpreadParticlesByCells();

        void AddParticleToCell(Cell* cell, int cellIndex, Model::ParticleIndex index);

        void RemoveParticleFromCell(Cell* cell, int cellIndex, Model::ParticleIndex index);

        DISALLOW_COPY_AND_ASSIGN(CellListNeighborProvider);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_CellListNeighborProvider_h */

