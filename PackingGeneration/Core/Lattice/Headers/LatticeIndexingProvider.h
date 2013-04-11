// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_LatticeIndexingProvider_h
#define Core_Lattice_Headers_LatticeIndexingProvider_h

#include "Core/Headers/Macros.h"
#include "ILatticeIndexingProvider.h"
namespace Core { class ILinearIndexingProvider; }

namespace Core
{
    //Represents a class for moving a lattice cell over the each node in the system.
    //This is a general iterator, which supports any type of indexing, but is not very fast.

    //Example: let's say we have the following nodes in 2D system and a 2DQ4 lattice:
    //3 7 11 15 19
    //2 6 10 14 18
    //1 5 9  13 17
    //0 4 8  12 16
    //Then for the node 9 the lattice cell will include nodes 10 13 8 5; after MoveNext method
    //the current node will be 10; cell nodes-11 14 9 6.
    class LatticeIndexingProvider : public ILatticeIndexingProvider
    {
    private:
        IGeometryParameters* geometryParameters;
        ILattice* lattice;
        ILinearIndexingProvider* linearIndexingProvider;

        DiscreteSpatialVector systemSize;

    public:
        LatticeIndexingProvider(ILattice* lattice, ILinearIndexingProvider* linearIndexingProvider);

        virtual ~LatticeIndexingProvider(void);

        OVERRIDE void SetGeometryParameters(IGeometryParameters* geometryParameters);

        OVERRIDE IGeometryParameters* GetGeometryParameters() const;

        OVERRIDE ILattice* GetLattice() const;

        // Gets the indexes of the nodes of the current lattice cell.
        // Returns the vector of size NumberOfLatticeVectors.
        OVERRIDE void FillCellNodeIndexes(const DiscreteSpatialVector& latticeCenterCoordinates, std::vector<int>* cellNodeIndexes) const;

        OVERRIDE int GetCellNodeIndex(const DiscreteSpatialVector& centerNodeCoordinates) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(LatticeIndexingProvider);
    };
}

#endif /* Core_Lattice_Headers_LatticeIndexingProvider_h */

