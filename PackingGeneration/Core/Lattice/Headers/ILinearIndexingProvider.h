// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Lattice_Headers_ILinearIndexingProvider_h
#define Core_Lattice_Headers_ILinearIndexingProvider_h

#include "Core/Headers/Types.h"
namespace Core { class IGeometryParameters; }

namespace Core
{
    //Represents an interface for linear indexing of nodes in a multidimensional space.
    class ILinearIndexingProvider
    {
    public:
        static const int UnavailableNodeIndex = -1;

        virtual void SetGeometryParameters(IGeometryParameters* geometryParameters) = 0;

        virtual IGeometryParameters* GetGeometryParameters() = 0;

        //Gets the linear index of a point by its multidimensional coordinates.
        //Negative coordinates or coordinates, exceeding the geometry size, may be passed for periodic dimensions.
        virtual int GetLinearIndex(const DiscreteSpatialVector& multidimensionalIndexes) const = 0;

        //Gets the multidimensional coordinates of a point by its linear index.
        virtual void FillMultidimensionalIndexes(int linearIndex, DiscreteSpatialVector* multidimensionalIndexes) const = 0;

        virtual ~ILinearIndexingProvider(){};
    };
}

#endif /* Core_Lattice_Headers_ILinearIndexingProvider_h */

