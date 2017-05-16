// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/GenericLattice.h"

using namespace std;

namespace Core
{
    GenericLattice::GenericLattice(int dimensions)
    {
        // If needed, pass them dynamically
        if (dimensions == 2)
        {
            currentLattice = &d2Lattice;
        }
        if (dimensions == 3)
        {
            currentLattice = &d3Lattice;
        }
    }

    int GenericLattice::GetLatticeSize() const
    {
        return currentLattice->GetLatticeSize();
    }

    int GenericLattice::GetDimensions() const
    {
        return currentLattice->GetDimensions();
    }

    void GenericLattice::FillLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        currentLattice->FillLatticeVector(latticeVectorIndex, latticeVector);
    }

    int GenericLattice::GetOppositeLatticeVectorIndex(int latticeVectorIndex) const
    {
        return currentLattice->GetOppositeLatticeVectorIndex(latticeVectorIndex);
    }

    void GenericLattice::FillOppositeLatticeVector(int latticeVectorIndex, DiscreteSpatialVector* latticeVector) const
    {
        currentLattice->FillOppositeLatticeVector(latticeVectorIndex, latticeVector);
    }
}
