// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_ColumnMajorIndexingProvider_h
#define Core_Lattice_Headers_ColumnMajorIndexingProvider_h

#include <cmath>
#include "ILinearIndexingProvider.h"
#include "Core/Headers/Types.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/Macros.h"

namespace Core
{
    //Provides linear indexing with the sample output for two-dimensional indexes m[i][j] 
    //(where i denotes row and X-coordinate, j--column and Y-coordinate, as for matrices) like that:
    //0 2 4
    //1 3 5
    //Remarks:
    //If the index i denotes the X-axis, it is directed downwards; the Y-axis is directed leftwards.
    //For ordinary axis directions the output looks like:
    //4 5
    //2 3
    //0 1 ->X
    //For the name, see wikipedia, Column Major Order.
    class ColumnMajorIndexingProvider : public ILinearIndexingProvider
    {
    private:
        IGeometryParameters* geometryParameters;

        //cached variables
        int nodesCount;
        DiscreteSpatialVector systemSize;

    public:
        ColumnMajorIndexingProvider();

        virtual ~ColumnMajorIndexingProvider();

        OVERRIDE void SetGeometryParameters(IGeometryParameters* geometryParameters);

        OVERRIDE IGeometryParameters* GetGeometryParameters();

        //Gets the linear index of a point by its multidimensional coordinates.
        //Let coordinate indexes be (7, 6, 8), sizes = {10, 10, 10}. We expect pointNumber to be 867.
        //Each coordinate index should span from 0 to sizes[dimension] - 1 for non-periodic dimensions; 
        //there are no restrictions for periodic ones.
        OVERRIDE int GetLinearIndex(const DiscreteSpatialVector& multidimensionalIndexes) const;

        //Gets the multidimensional coordinates of a point by its linear index.
        //Let linearIndex = 867, numberOfDimensions = 3, sizes = {10, 10, 10}.
        //We expect coordinate indexes to be (7, 6, 8).
        //Each coordinate spans from 0 to sizes[dimension] - 1.
        OVERRIDE void FillMultidimensionalIndexes(int linearIndex, DiscreteSpatialVector* multidimensionalIndexes) const;

    private:
        int ApplyPeriodicity(int coordinate, Axis::Type dimension) const;

        DISALLOW_COPY_AND_ASSIGN(ColumnMajorIndexingProvider);
    };
}

#endif /* Core_Lattice_Headers_ColumnMajorIndexingProvider_h */

