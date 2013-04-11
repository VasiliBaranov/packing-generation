// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Geometry_Headers_GeometryParameters_h
#define Core_Geometry_Headers_GeometryParameters_h

#include "Core/Headers/Macros.h"
#include "Core/Headers/Types.h"
#include "IGeometryParameters.h"

namespace Core
{
    // Represents geometry parameters for defining local or global system geometries.
    class GeometryParameters : public virtual IGeometryParameters
    {
    private:
        bool periodicity[DIMENSIONS];
        DiscreteSpatialVector systemSize;

    public:
        GeometryParameters();

        virtual ~GeometryParameters(void);

        OVERRIDE int GetNodesCount() const;

        OVERRIDE void FillSystemSize(DiscreteSpatialVector* systemSize) const;

        OVERRIDE int GetSystemSize(int dimension) const;

        OVERRIDE void SetSystemSize(int dimension, int size);

        // Gets the value, indicating whether the system is periodic by the given axis.
        OVERRIDE bool IsPeriodicByAxis(int dimension) const;

        // Sets the value, indicating whether the system is periodic by the given axis.
        OVERRIDE void SetPeriodicity(int dimension, bool isPeriodic);

    private:
        DISALLOW_COPY_AND_ASSIGN(GeometryParameters);
    };
}

#endif /* Core_Geometry_Headers_GeometryParameters_h */

