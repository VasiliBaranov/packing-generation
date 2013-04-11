// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/D3Q27Lattice.h"

using namespace std;

namespace Core
{
    D3Q27Lattice::D3Q27Lattice()
    {
        InitializeLatticeVectors();
        InitializeOppositeLatticeVectorIndexes();
    }

    int D3Q27Lattice::GetLatticeSize() const
    {
        return LatticeSize;
    }

    int D3Q27Lattice::GetDimensions() const
    {
        return dimensions;
    }

    void D3Q27Lattice::FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        std::copy(latticeVectors[latticeVectorIndex], latticeVectors[latticeVectorIndex] + dimensions, latticeVector->begin());
    }

    int D3Q27Lattice::GetOppositeLatticeVectorIndex(int latticeVectorIndex) const
    {
        return oppositeLatticeVectorIndexes[latticeVectorIndex];
    }

    void D3Q27Lattice::FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        int oppositeVectorIndex = oppositeLatticeVectorIndexes[latticeVectorIndex];
        std::copy(latticeVectors[oppositeVectorIndex], latticeVectors[oppositeVectorIndex] + dimensions, latticeVector->begin());
    }

    void D3Q27Lattice::InitializeLatticeVectors()
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

        //Lattice vectors for the particles in XZ-plane
        latticeVectors[9][0] =  0; latticeVectors[9][1] = 0;  latticeVectors[9][2] = 1;
        latticeVectors[10][0] =  1; latticeVectors[10][1] =  0;  latticeVectors[10][2] = 1;
        latticeVectors[11][0] =  1; latticeVectors[11][1] =  1;  latticeVectors[11][2] = 1;
        latticeVectors[12][0] =  0; latticeVectors[12][1] =  1;  latticeVectors[12][2] = 1;
        latticeVectors[13][0] = -1; latticeVectors[13][1] =  1;  latticeVectors[13][2] = 1;
        latticeVectors[14][0] = -1; latticeVectors[14][1] =  0;  latticeVectors[14][2] = 1;
        latticeVectors[15][0] = -1; latticeVectors[15][1] = -1;  latticeVectors[15][2] = 1;
        latticeVectors[16][0] =  0; latticeVectors[16][1] = -1;  latticeVectors[16][2] = 1;
        latticeVectors[17][0] =  1; latticeVectors[17][1] = -1;  latticeVectors[17][2] = 1;

        //Lattice vectors for the particles in YZ-plane
        latticeVectors[18][0] =  0; latticeVectors[18][1] =  0;  latticeVectors[18][2] = -1;
        latticeVectors[19][0] =  1; latticeVectors[19][1] =  0;  latticeVectors[19][2] = -1;
        latticeVectors[20][0] =  1; latticeVectors[20][1] =  1;  latticeVectors[20][2] = -1;
        latticeVectors[21][0] =  0; latticeVectors[21][1] =  1;  latticeVectors[21][2] = -1;
        latticeVectors[22][0] = -1; latticeVectors[22][1] =  1;  latticeVectors[22][2] = -1;
        latticeVectors[23][0] = -1; latticeVectors[23][1] =  0;  latticeVectors[23][2] = -1;
        latticeVectors[24][0] = -1; latticeVectors[24][1] = -1;  latticeVectors[24][2] = -1;
        latticeVectors[25][0] =  0; latticeVectors[25][1] = -1;  latticeVectors[25][2] = -1;
        latticeVectors[26][0] =  1; latticeVectors[26][1] = -1;  latticeVectors[26][2] = -1;
    }

    void D3Q27Lattice::InitializeOppositeLatticeVectorIndexes()
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

        //Lattice vectors for the particles in XZ-plane
        oppositeLatticeVectorIndexes[9] = 18;
        oppositeLatticeVectorIndexes[10] = 23;
        oppositeLatticeVectorIndexes[11] = 24;
        oppositeLatticeVectorIndexes[12] = 25;
        oppositeLatticeVectorIndexes[13] = 26;
        oppositeLatticeVectorIndexes[14] = 19;
        oppositeLatticeVectorIndexes[15] = 20;
        oppositeLatticeVectorIndexes[16] = 21;
        oppositeLatticeVectorIndexes[17] = 22;

        //Lattice vectors for the particles in YZ-plane
        oppositeLatticeVectorIndexes[18] = 9;
        oppositeLatticeVectorIndexes[19] = 14;
        oppositeLatticeVectorIndexes[20] = 15;
        oppositeLatticeVectorIndexes[21] = 16;
        oppositeLatticeVectorIndexes[22] = 17;
        oppositeLatticeVectorIndexes[23] = 10;
        oppositeLatticeVectorIndexes[24] = 11;
        oppositeLatticeVectorIndexes[25] = 12;
        oppositeLatticeVectorIndexes[26] = 13;
    }
}
