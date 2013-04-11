// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Geometries_Headers_BaseGeometry_h
#define Generation_Geometries_Headers_BaseGeometry_h

#include "IGeometry.h"
namespace Model { class SystemConfig; }

namespace Geometries
{
    // Represents a base geometry of a tube, periodic along its direction.
    class BaseGeometry : public IGeometry
    {
    protected:
        const Model::SystemConfig* config;
        Core::SpatialVector packingHalfSize;

    public:
        explicit BaseGeometry(const Model::SystemConfig& config);

        virtual ~BaseGeometry();

    protected:
        void EnsureVerticalPeriodicity(Core::SpatialVector* x, Core::SpatialVector* y, int dimension) const;

        void EnsureVerticalPeriodicityAfterRepulsion(Core::SpatialVector* x, int dimension) const;
    };
}


#endif /* Generation_Geometries_Headers_BaseGeometry_h */
