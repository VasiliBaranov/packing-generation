// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Geometry_Headers_IGeometryParameters_h
#define Core_Geometry_Headers_IGeometryParameters_h

#include "Core/Headers/Types.h"

namespace Core
{
    //Defines methods for system geometry parameters.
    //All occurrences of "Axis axis" where replaced with "int dimension",
    //as it removes dependency over 3D geometry.
    class IGeometryParameters
    {
    public:
        virtual void FillSystemSize(DiscreteSpatialVector* systemSize) const = 0;

        virtual int GetSystemSize(int dimension) const = 0;

        virtual void SetSystemSize(int dimension, int size) = 0;

        //Gets the value, indicating whether the system is periodic by the given axis.
        virtual bool IsPeriodicByAxis(int dimension) const = 0;

        //Sets the value, indicating whether the system is periodic by the given axis.
        virtual void SetPeriodicity(int dimension, bool isPeriodic) = 0;

        virtual int GetNodesCount() const = 0;

        virtual ~IGeometryParameters(){};
    };
}

#endif /* Core_Geometry_Headers_IGeometryParameters_h */

