// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Geometries_Headers_CircleGeometry_h
#define Generation_Geometries_Headers_CircleGeometry_h

#include "BaseGeometry.h"
#include "Core/Headers/Macros.h"

namespace Geometries
{
    // Represents a tube with circular cross-section.
    class CircleGeometry : public BaseGeometry
    {
    private:
        // TODO: add similar shifts into other non-bulk geometries. Or may be created ShiftedGeometry wrapper
        // (may be templated not to decrease performance on virtual calls;
        // but it will anyway subtract the shift before and add the shift after calling the base Geometry methods; this is the main performance drawback).
        // Or maybe create pairs of classes CircleGeometry - ShiftedCircleGeometry. Or add a template parameter to CircleGeometry<TSupportsShift> (this is the best variant).
        Core::SpatialVector shift;

    public:
        explicit CircleGeometry(const Model::SystemConfig& config);

        CircleGeometry(const Model::SystemConfig& config, const Core::SpatialVector& shift);

        OVERRIDE Core::FLOAT_TYPE GetTotalVolume() const;

        OVERRIDE void EnsurePeriodicConditions(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle) const;

        OVERRIDE void EnsureBoundaries(const Model::DomainParticle& initialParticle, Model::DomainParticle* movedParticle, Core::FLOAT_TYPE minNormalizedDistance) const;

        OVERRIDE bool IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const;

        virtual ~CircleGeometry();

    private:
        DISALLOW_COPY_AND_ASSIGN(CircleGeometry);
    };
}


#endif /* Generation_Geometries_Headers_CircleGeometry_h */

