// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Geometries_Headers_IGeometry_h
#define Generation_Geometries_Headers_IGeometry_h

#include "Generation/Model/Headers/Types.h"

namespace Geometries
{
    // Defines methods for specific geometries handling.
    class IGeometry
    {
    public:
        virtual Core::FLOAT_TYPE GetTotalVolume() const = 0;

        virtual void EnsurePeriodicConditions(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle) const = 0;

        virtual void EnsureBoundaries(const Model::DomainParticle& initialParticle, Model::DomainParticle* movedParticle, Core::FLOAT_TYPE minNormalizedDistance) const = 0;

        virtual bool IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const = 0;

        virtual ~IGeometry(){ };
    };
}

#endif /* Generation_Geometries_Headers_IGeometry_h */

