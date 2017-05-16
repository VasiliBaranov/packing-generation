// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_ILattice_h
#define Core_Lattice_Headers_ILattice_h

#include "Core/Headers/Types.h"

namespace Core
{
    //Defines methods for a lattice stencil.
    class ILattice
    {
    public:

        virtual int GetLatticeSize() const = 0;

        virtual int GetDimensions() const = 0;

        //Gets the lattice vector by the index.
        virtual void FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const = 0;

        //Gets the lattice vector index for the vector, opposite to the one with the given index.
        virtual int GetOppositeLatticeVectorIndex(int latticeVectorIndex) const = 0;

        //Gets the lattice vector, which is opposite to the vector with the given index.
        virtual void FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const = 0;

        virtual ~ILattice(){};
    };
}

#endif /* Core_Lattice_Headers_ILattice_h */

