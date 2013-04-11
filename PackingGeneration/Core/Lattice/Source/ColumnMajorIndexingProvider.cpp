// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ColumnMajorIndexingProvider.h"
#include "Core/Geometry/Headers/IGeometryParameters.h"

using namespace std;

namespace Core
{
    ColumnMajorIndexingProvider::ColumnMajorIndexingProvider()
    {
    }

    ColumnMajorIndexingProvider::~ColumnMajorIndexingProvider()
    {
    }

    void ColumnMajorIndexingProvider::SetGeometryParameters(IGeometryParameters* geometryParameters)
    {
        this->geometryParameters = geometryParameters;

        //cache variables
        nodesCount = geometryParameters->GetNodesCount();
        geometryParameters->FillSystemSize(&systemSize);
    }

    //Gets the available volume size.
    IGeometryParameters* ColumnMajorIndexingProvider::GetGeometryParameters()
    {
        return geometryParameters;
    }

    //Gets the linear index of a point by its multidimensional coordinates.
    //Let coordinate indexes be (7, 6, 8), sizes = {10, 10, 10}. We expect pointNumber to be 867.
    //each coordinate index spans from 0 to sizes[dimension] - 1.
    int ColumnMajorIndexingProvider::GetLinearIndex(const DiscreteSpatialVector& multidimensionalIndexes) const
    {
        int linearIndex = 0;
        int multiplier = 1;
        int addition;
        for (int dimension = 0; dimension < DIMENSIONS; dimension++)
        {
            int coordinate = ApplyPeriodicity(multidimensionalIndexes[dimension], static_cast<Axis::Type>(dimension));
            if (coordinate == ILinearIndexingProvider::UnavailableNodeIndex)
            {
                return ILinearIndexingProvider::UnavailableNodeIndex;
            }

            //Addition = 7
            addition = multiplier * coordinate;
            linearIndex += addition;

            //multiplier = 10; The next addition will be 60
            multiplier = multiplier * systemSize[dimension];
        }

        return linearIndex;
    }

    //Gets the multidimensional coordinates of a point by its linear index.
    //Let linearIndex = 867, numberOfDimensions = 3, sizes = {10, 10, 10}.
    //We expect coordinate indexes to be (7, 6, 8).
    //Each coordinate spans from 0 to sizes[dimension] - 1.
    void ColumnMajorIndexingProvider::FillMultidimensionalIndexes(int linearIndex, DiscreteSpatialVector* multidimensionalIndexes) const
    {
        int currentLinearIndex = linearIndex;
        DiscreteSpatialVector& multidimensionalIndexesRef = *multidimensionalIndexes;

        for (int dimension = 0; dimension < DIMENSIONS; dimension++)
        {
            //multidimensionalIndex = 867 mod 10 = 7
            multidimensionalIndexesRef[dimension] = currentLinearIndex % systemSize[dimension];

            //currentLinearIndex = 86, so the next index will be 6
            // Integer division is equivalent to floor.
            currentLinearIndex = currentLinearIndex / systemSize[dimension];
        }
    }

    int ColumnMajorIndexingProvider::ApplyPeriodicity(int coordinate, Axis::Type dimension) const
    {
        if (coordinate >= 0 && coordinate < systemSize[dimension])
        {
            return coordinate;
        }

        if (!geometryParameters->IsPeriodicByAxis(dimension))
        {
            return ILinearIndexingProvider::UnavailableNodeIndex;
        }

        //let coordinate be -11, size in the dimension = 10
        if (coordinate < 0)
        {
            coordinate = - coordinate; //coordinate = 11
            coordinate %= systemSize[dimension]; //coordinate = 1
            if (coordinate != 0)
            {
                coordinate = systemSize[dimension] - coordinate; //coordinate = 9
            }
        }
        else
        {
            coordinate %= systemSize[dimension];
        }

        return coordinate;
    }
}

