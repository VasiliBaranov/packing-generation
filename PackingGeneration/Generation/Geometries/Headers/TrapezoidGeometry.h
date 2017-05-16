// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Geometries_Headers_TrapezoidGeometry_h
#define Generation_Geometries_Headers_TrapezoidGeometry_h

#include "BaseGeometry.h"
#include "Core/Headers/Macros.h"

namespace Geometries
{
    // Represents a tube with trapezoid cross-section.
    class TrapezoidGeometry : public BaseGeometry
    {
    private:
        double icos_alpha;
        double a_trapezoid;
        double a_div_y_trapezoid;
        double x_left_trapezoid, x_right_trapezoid;

    public:
        explicit TrapezoidGeometry(const Model::SystemConfig& config);

        OVERRIDE Core::FLOAT_TYPE GetTotalVolume() const;

        OVERRIDE void EnsurePeriodicConditions(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle) const;

        OVERRIDE void EnsureBoundaries(const Model::DomainParticle& initialParticle, Model::DomainParticle* movedParticle, Core::FLOAT_TYPE minNormalizedDistance) const;

        OVERRIDE bool IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const;

        virtual ~TrapezoidGeometry();

    private:
        DISALLOW_COPY_AND_ASSIGN(TrapezoidGeometry);
    };
}

#endif /* Generation_Geometries_Headers_TrapezoidGeometry_h */

