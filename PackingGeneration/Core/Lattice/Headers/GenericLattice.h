// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_GenericLattice_h
#define Core_Lattice_Headers_GenericLattice_h

#include "ILattice.h"
#include "../Headers/D2Q9Lattice.h"
#include "../Headers/D3Q27Lattice.h"

namespace Core
{
    //Represents a lattice for two and three dimensions.
    class GenericLattice : public ILattice
    {
    private:
        D2Q9Lattice d2Lattice;
        D3Q27Lattice d3Lattice;
        ILattice* currentLattice;

    public:
        GenericLattice(int dimensions);

        OVERRIDE int GetLatticeSize() const;

        OVERRIDE int GetDimensions() const;

        OVERRIDE void FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const;

        OVERRIDE int GetOppositeLatticeVectorIndex(int latticeVectorIndex) const;

        OVERRIDE void FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const;

        DISALLOW_COPY_AND_ASSIGN(GenericLattice);
    };
}

#endif /* Core_Lattice_Headers_GenericLattice_h */

