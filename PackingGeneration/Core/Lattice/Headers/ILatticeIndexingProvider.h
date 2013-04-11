// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_ILatticeIndexingProvider_h
#define Core_Lattice_Headers_ILatticeIndexingProvider_h

#include <vector>
#include "Core/Headers/Types.h"
namespace Core { class IGeometryParameters; }
namespace Core { class ILattice; }

namespace Core
{
    //Represents an interface for moving a lattice stencil over each node in the system.
    //In Mpi environment movement takes place just by the non-ghost cells of the local system state.
    //See wikipedia, Iterator design pattern.
    class ILatticeIndexingProvider
    {
    public:

        virtual void SetGeometryParameters(IGeometryParameters* geometryParameters) = 0;

        virtual IGeometryParameters* GetGeometryParameters() const = 0;

        virtual ILattice* GetLattice() const = 0;

        // Gets the indexes of the nodes of the current lattice cell.
        // Returns the vector of size NumberOfLatticeVectors.
        virtual void FillCellNodeIndexes(const DiscreteSpatialVector& latticeCenterCoordinates, std::vector<int>* cellNodeIndexes) const = 0;

        virtual int GetCellNodeIndex(const DiscreteSpatialVector& centerNodeCoordinates) const = 0;

        virtual ~ILatticeIndexingProvider(){};
    };
}

#endif /* Core_Lattice_Headers_ILatticeIndexingProvider_h */

