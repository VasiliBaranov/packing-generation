// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/LatticeIndexingProvider.h"
#include "Core/Geometry/Headers/IGeometryParameters.h"
#include "../Headers/ILinearIndexingProvider.h"
#include "../Headers/ILattice.h"
#include "Core/Headers/VectorUtilities.h"

using namespace std;

namespace Core
{
    LatticeIndexingProvider::LatticeIndexingProvider(ILattice* lattice, ILinearIndexingProvider* linearIndexingProvider)
    {
        this->lattice = lattice;
        this->linearIndexingProvider = linearIndexingProvider;
    }

    LatticeIndexingProvider::~LatticeIndexingProvider(void)
    {
    }

    void LatticeIndexingProvider::SetGeometryParameters(IGeometryParameters* geometryParameters)
    {
        this->geometryParameters = geometryParameters;
        geometryParameters->FillSystemSize(&systemSize);
        linearIndexingProvider->SetGeometryParameters(geometryParameters);
    }

    IGeometryParameters* LatticeIndexingProvider::GetGeometryParameters() const
    {
        return geometryParameters;
    }

    ILattice* LatticeIndexingProvider::GetLattice() const
    {
        return lattice;
    }

    //Gets the indexes of the nodes of the current lattice cell.
    //Returns the vector of size NumberOfLatticeVectors.
    void LatticeIndexingProvider::FillCellNodeIndexes(const DiscreteSpatialVector& latticeCenterCoordinates, vector<int>* cellNodeIndexes) const
    {
        DiscreteSpatialVector cellNodeCoordinates;
        DiscreteSpatialVector latticeVector;

        int latticeSize = lattice->GetLatticeSize();
        cellNodeIndexes->resize(latticeSize);
        vector<int>& cellNodeIndexesRef = *cellNodeIndexes;
        for (int i = 0; i < latticeSize; ++i)
        {
            lattice->FillLatticeVector(i, &latticeVector);
            VectorUtilities::Add(latticeCenterCoordinates, latticeVector, &cellNodeCoordinates);
            cellNodeIndexesRef[i] = linearIndexingProvider->GetLinearIndex(cellNodeCoordinates);
        }
    }

    int LatticeIndexingProvider::GetCellNodeIndex(const DiscreteSpatialVector& centerNodeCoordinates) const
    {
        return linearIndexingProvider->GetLinearIndex(centerNodeCoordinates);
    }
}

