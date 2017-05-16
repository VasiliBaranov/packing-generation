// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/D2Q9Lattice.h"

using namespace std;

namespace Core
{
    D2Q9Lattice::D2Q9Lattice()
    {
        InitializeLatticeVectors();
        InitializeOppositeLatticeVectorIndexes();
    }

    int D2Q9Lattice::GetLatticeSize() const
    {
        return LatticeSize;
    }

    int D2Q9Lattice::GetDimensions() const
    {
        return dimensions;
    }

    void D2Q9Lattice::FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        std::copy(latticeVectors[latticeVectorIndex], latticeVectors[latticeVectorIndex] + dimensions, latticeVector->begin());
    }

    int D2Q9Lattice::GetOppositeLatticeVectorIndex(int latticeVectorIndex) const
    {
        return oppositeLatticeVectorIndexes[latticeVectorIndex];
    }

    void D2Q9Lattice::FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        int oppositeVectorIndex = oppositeLatticeVectorIndexes[latticeVectorIndex];
        std::copy(latticeVectors[oppositeVectorIndex], latticeVectors[oppositeVectorIndex] + dimensions, latticeVector->begin());
    }

    void D2Q9Lattice::InitializeLatticeVectors()
    {
        //Lattice vectors for the particles in XY-plane
        latticeVectors[0][0] =  0; latticeVectors[0][1] =  0;  latticeVectors[0][2] = 0;
        latticeVectors[1][0] =  1; latticeVectors[1][1] =  0;  latticeVectors[1][2] = 0;
        latticeVectors[2][0] =  1; latticeVectors[2][1] =  1;  latticeVectors[2][2] = 0;
        latticeVectors[3][0] =  0; latticeVectors[3][1] =  1;  latticeVectors[3][2] = 0;
        latticeVectors[4][0] = -1; latticeVectors[4][1] =  1;  latticeVectors[4][2] = 0;
        latticeVectors[5][0] = -1; latticeVectors[5][1] =  0;  latticeVectors[5][2] = 0;
        latticeVectors[6][0] = -1; latticeVectors[6][1] = -1;  latticeVectors[6][2] = 0;
        latticeVectors[7][0] =  0; latticeVectors[7][1] = -1;  latticeVectors[7][2] = 0;
        latticeVectors[8][0] =  1; latticeVectors[8][1] = -1;  latticeVectors[8][2] = 0;
    }

    void D2Q9Lattice::InitializeOppositeLatticeVectorIndexes()
    {
        //Lattice vectors for the particles in XY-plane
        oppositeLatticeVectorIndexes[0] = 0;
        oppositeLatticeVectorIndexes[1] = 5;
        oppositeLatticeVectorIndexes[2] = 6;
        oppositeLatticeVectorIndexes[3] = 7;
        oppositeLatticeVectorIndexes[4] = 8;
        oppositeLatticeVectorIndexes[5] = 1;
        oppositeLatticeVectorIndexes[6] = 2;
        oppositeLatticeVectorIndexes[7] = 3;
        oppositeLatticeVectorIndexes[8] = 4;
    }
}
