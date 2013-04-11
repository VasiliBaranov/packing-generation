// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_CellListNeighborProvider_h
#define Generation_PackingServices_DistanceServices_Headers_CellListNeighborProvider_h

#include "Core/Geometry/Headers/GeometryParameters.h"
#include "Core/Lattice/Headers/LatticeIndexingProvider.h"
#include "Core/Lattice/Headers/D3Q27Lattice.h"
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
    // Better data structures are octtree, loose octtree. But when using Verlet lists as a decorator of this neighbor provider the alternatives are not crucial.
    class CellListNeighborProvider : public virtual INeighborProvider
    {
    private:
        // Represents a single cell in the spatial domain, tracking particles belonging to this cell
        struct Cell : public Model::CubicBox
        {
            std::vector<Model::ParticleIndex> particleIndexes;
            std::vector<int> neighborCellIndexes; // for each cell index stores all its neighbors
            std::vector<Model::ParticleIndex> particleIndexesPermutation; // particleIndexesPermutation[i] gets the index of the particle i in the particleIndexes array

            Cell()
            {
                particleIndexes.reserve(50);
                neighborCellIndexes.reserve(50);
                particleIndexesPermutation.reserve(50);
            }
        };

        // Services
        // Declare as pointers and pass to the constructor if you need to make them replaceable (primarily for tests).
        Core::GeometryParameters geometryParameters;
        Core::D3Q27Lattice lattice;
        Core::ColumnMajorIndexingProvider linearIndexingProvider;
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

