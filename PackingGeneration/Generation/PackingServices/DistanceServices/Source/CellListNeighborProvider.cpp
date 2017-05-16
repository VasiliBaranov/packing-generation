// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CellListNeighborProvider.h"

#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"

using namespace Model;
using namespace Core;
using namespace std;

namespace PackingServices
{
    CellListNeighborProvider::CellListNeighborProvider(GeometryService* geometryService, GeometryCollisionService* geometryCollisionService)
    : lattice(DIMENSIONS),
      latticeIndexingProvider(&lattice, &linearIndexingProvider)
    {
        this->geometryService = geometryService;
        this->geometryCollisionService = geometryCollisionService;
    }

    void CellListNeighborProvider::SetContext(const ModellingContext& context)
    {
        config = context.config;
        geometryService->SetContext(context);
    }

    CellListNeighborProvider::~CellListNeighborProvider()
    {
    }

    void CellListNeighborProvider::SetParticles(const Packing& particles)
    {
        this->particles = &particles;

        InitializeCellDimensions();

        InitializeDomainCells();

        SpreadParticlesByCells();
    }

    FLOAT_TYPE CellListNeighborProvider::GetTimeToUpdateBoundary(ParticleIndex particleIndex, const SpatialVector& point, const SpatialVector& velocity) const
    {
        DiscreteSpatialVector latticePoint;
        FillDomainCellCoordinates(point, &latticePoint);
        int linearCellIndex = latticeIndexingProvider.GetCellNodeIndex(latticePoint);
        const Cell& cell = domainCells[linearCellIndex];

        FLOAT_TYPE intersectionTime;
        int intersectionWallIndex;
        geometryCollisionService->FindIntersection(point, velocity, cell, -1, &intersectionTime, &intersectionWallIndex);

        return intersectionTime;
    }

    void CellListNeighborProvider::SpreadParticlesByCells()
    {
        const Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DiscreteSpatialVector latticePoint;
            const DomainParticle& particle = particlesRef[particleIndex];
            FillDomainCellCoordinates(particle.coordinates, &latticePoint);
            int cellIndex = latticeIndexingProvider.GetCellNodeIndex(latticePoint);
            Cell* cell = &domainCells[cellIndex];
            for (vector<int>::size_type i = 0; i < cell->neighborCellIndexes.size(); ++i)
            {
                int neighborCellIndex = cell->neighborCellIndexes[i];
                Cell* neighborCell = &domainCells[neighborCellIndex];

                AddParticleToCell(neighborCell, neighborCellIndex, particleIndex);
            }
        }
    }

    void CellListNeighborProvider::InitializeCellDimensions()
    {
        FLOAT_TYPE meanDiameter = geometryService->GetMaxParticleDiameter(*particles);

        SpatialVector floatCellCounts;
        VectorUtilities::DivideByValue(config->packingSize, meanDiameter, &floatCellCounts);
        VectorUtilities::Floor<SpatialVector, SpatialVector>(floatCellCounts, &floatCellCounts);
        VectorUtilities::Convert<SpatialVector, DiscreteSpatialVector>(floatCellCounts, &cellsCounts);

        VectorUtilities::Divide(config->packingSize, cellsCounts, &cellSize);
    }

    void CellListNeighborProvider::InitializeDomainCells()
    {
        int totalCellCount = VectorUtilities::GetProduct(cellsCounts);
        domainCells.clear();
        domainCells.resize(totalCellCount);

        for (int i = 0; i < DIMENSIONS; ++i)
        {
            geometryParameters.SetSystemSize(i, cellsCounts[i]);
            geometryParameters.SetPeriodicity(i, true);
        }

        linearIndexingProvider.SetGeometryParameters(&geometryParameters);
        latticeIndexingProvider.SetGeometryParameters(&geometryParameters);

        for (int i = 0; i < totalCellCount; ++i)
        {
            Cell* cell = &domainCells[i];

            DiscreteSpatialVector latticePoint;
            linearIndexingProvider.FillMultidimensionalIndexes(i, &latticePoint);
            latticeIndexingProvider.FillCellNodeIndexes(latticePoint, &cell->neighborCellIndexes);

            // Neighbor cell indexes may not be unique in the only case: if the cells count at least by one dimension is less than 3.
            StlUtilities::SortAndResizeToUnique(&cell->neighborCellIndexes);

            SpatialVector minVertexCoordinates;
            VectorUtilities::Multiply(latticePoint, cellSize, &minVertexCoordinates);
            cell->Initialize(minVertexCoordinates, cellSize);
        }
    }

    const ParticleIndex* CellListNeighborProvider::GetNeighborIndexes(ParticleIndex particleIndex, ParticleIndex* neighborsCount) const
    {
        const Packing& particlesRef = *particles;
        ParticleIndex& neighborsCountRef = *neighborsCount;
        int cellIndex;
        const DomainParticle* particle = &particlesRef[particleIndex];
        ParticleIndex* neighborIndexes = GetNeighborIndexes(particle->coordinates, neighborsCount, &cellIndex);

        // We move the particleIndex to the neighborIndexes end, so that the particle is still in the given cell (as neighborIndexes is cell.particleIndexes array),
        // but decrease neighborsCount, so that the particleIndex is not included in the results (which is correct by semantics).
        Cell* cell = &domainCells[cellIndex];
        int localParticleIndex = cell->particleIndexesPermutation[particleIndex]; // the localParticleIndex should always be valid, as particle should reside in the given cell
        int localNeighborIndex = neighborsCountRef - 1;
        if (localParticleIndex < localNeighborIndex)
        {
            ParticleIndex neighborIndex = neighborIndexes[localNeighborIndex];
            neighborIndexes[localParticleIndex] = neighborIndex;
            neighborIndexes[localNeighborIndex] = particleIndex;

            cell->particleIndexesPermutation[neighborIndex] = localParticleIndex;
            cell->particleIndexesPermutation[particleIndex] = localNeighborIndex;
        }
        neighborsCountRef--;

        return neighborIndexes;
    }

    const ParticleIndex* CellListNeighborProvider::GetNeighborIndexes(const SpatialVector& coordinates, ParticleIndex* neighborsCount) const
    {
        int linearCellIndex;
        return GetNeighborIndexes(coordinates, neighborsCount, &linearCellIndex);
    }

    ParticleIndex* CellListNeighborProvider::GetNeighborIndexes(const SpatialVector& coordinates, ParticleIndex* neighborsCount, int* linearCellIndex) const
    {
        DiscreteSpatialVector latticePoint;
        FillDomainCellCoordinates(coordinates, &latticePoint);

        *linearCellIndex = latticeIndexingProvider.GetCellNodeIndex(latticePoint);

        Cell& cell = domainCells[*linearCellIndex];
        *neighborsCount = cell.particleIndexes.size();
        return &cell.particleIndexes[0];
    }

    void CellListNeighborProvider::EndMove()
    {
        const Packing& particlesRef = *particles;
        DiscreteSpatialVector latticePoint;
        const DomainParticle* particle = &particlesRef[movedParticleIndex];
        FillDomainCellCoordinates(particle->coordinates, &latticePoint);

        bool cellNotChanged = StlUtilities::Equals(latticePoint, previousLatticePoint);
        if (cellNotChanged)
        {
            return;
        }

        int previousCellIndex = latticeIndexingProvider.GetCellNodeIndex(previousLatticePoint);
        Cell* previousCell = &domainCells[previousCellIndex];
        for (vector<int>::size_type i = 0; i < previousCell->neighborCellIndexes.size(); ++i)
        {
            int cellIndex = previousCell->neighborCellIndexes[i];
            Cell* neighborCell = &domainCells[cellIndex];
            RemoveParticleFromCell(neighborCell, cellIndex, movedParticleIndex);
        }

        int currentCellIndex = latticeIndexingProvider.GetCellNodeIndex(latticePoint);
        Cell* currentCell = &domainCells[currentCellIndex];
        for (vector<int>::size_type i = 0; i < currentCell->neighborCellIndexes.size(); ++i)
        {
            int cellIndex = currentCell->neighborCellIndexes[i];
            Cell* neighborCell = &domainCells[cellIndex];
            AddParticleToCell(neighborCell, cellIndex, movedParticleIndex);
        }
    }

    void CellListNeighborProvider::StartMove(ParticleIndex particleIndex)
    {
        movedParticleIndex = particleIndex;

        const Packing& particlesRef = *particles;
        const DomainParticle* particle = &particlesRef[particleIndex];
        FillDomainCellCoordinates(particle->coordinates, &previousLatticePoint);
    }

    void CellListNeighborProvider::FillDomainCellCoordinates(const SpatialVector& point, DiscreteSpatialVector* latticePoint) const
    {
        SpatialVector floatLatticePoint;
        VectorUtilities::Divide(point, cellSize, &floatLatticePoint);
        VectorUtilities::Floor(floatLatticePoint, &floatLatticePoint);
        VectorUtilities::Convert<SpatialVector, DiscreteSpatialVector>(floatLatticePoint, latticePoint);

        DiscreteSpatialVector& latticePointRef = *latticePoint;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            if (latticePointRef[i] < 0)
            {
                latticePointRef[i] = 0;
            }
            if (latticePointRef[i] >= cellsCounts[i])
            {
                latticePointRef[i] = cellsCounts[i] - 1;
            }
        }
    }

    void CellListNeighborProvider::AddParticleToCell(Cell* cell, int cellIndex, ParticleIndex particleIndex)
    {
        cell->particleIndexes.push_back(particleIndex);
        cell->particleIndexesPermutation[particleIndex] = cell->particleIndexes.size() - 1;
    }

    void CellListNeighborProvider::RemoveParticleFromCell(Cell* cell, int cellIndex, ParticleIndex particleIndex)
    {
        ParticleIndex indexInCell = cell->particleIndexesPermutation[particleIndex];

        if (indexInCell < static_cast<int>(cell->particleIndexes.size() - 1))
        {
            StlUtilities::QuicklyRemoveAt(&cell->particleIndexes, indexInCell);

            // Update the moved particle index in the cell.
            // Just one (the last one) particle will be moved to the deleted particle position.
            ParticleIndex movedParticleIndex = cell->particleIndexes[indexInCell];
            cell->particleIndexesPermutation[movedParticleIndex] = indexInCell;
            cell->particleIndexesPermutation.erase(particleIndex);
        }
        else // indexInCell is the last particle, so we can simply remove it
        {
            cell->particleIndexes.pop_back();
            cell->particleIndexesPermutation.erase(particleIndex);
        }
    }
}

