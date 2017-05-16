// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_D2Q9Lattice_h
#define Core_Lattice_Headers_D2Q9Lattice_h

#include "ILattice.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/Macros.h"

namespace Core
{
    //Represents a D2Q9 lattice.
    class D2Q9Lattice : public ILattice
    {
    public:
        static const int LatticeSize = 9;
    private:
        static const int dimensions = 2;

        int latticeVectors[LatticeSize][dimensions];

        //Stores indexes of the opposite lattice vectors. For example, if the lattice vector 6 is oppoiste to the vector 1,
        //then oppositeLatticeVectorIndexes[1] = 6.
        int oppositeLatticeVectorIndexes[LatticeSize];

        //I've moved lattice vector weights access from this class to equilibrium distribution provider,
        //as weights are more specific to the equilibrium distribution provider, and may be changed independently of the lattice.

    public:
        D2Q9Lattice();

        OVERRIDE int GetLatticeSize() const;

        OVERRIDE int GetDimensions() const;

        OVERRIDE void FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const;

        OVERRIDE int GetOppositeLatticeVectorIndex(int latticeVectorIndex) const;

        OVERRIDE void FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const;

    private:
        void InitializeLatticeVectors();

        void InitializeOppositeLatticeVectorIndexes();

        DISALLOW_COPY_AND_ASSIGN(D2Q9Lattice);
    };
}

#endif /* Core_Lattice_Headers_D2Q9Lattice_h */

