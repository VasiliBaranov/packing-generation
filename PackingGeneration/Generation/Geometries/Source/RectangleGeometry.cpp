// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/RectangleGeometry.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Exceptions.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    RectangleGeometry::RectangleGeometry(const SystemConfig& config) : BaseGeometry(config)
    {
        throw NotImplementedException("Check code");
    }

    RectangleGeometry::~RectangleGeometry()
    {

    }

    FLOAT_TYPE RectangleGeometry::GetTotalVolume() const
    {
        return VectorUtilities::GetProduct(config->packingSize);
    }

    void RectangleGeometry::EnsurePeriodicConditions(DomainParticle* firstParticle, DomainParticle* secondParticle) const
    {
        EnsureVerticalPeriodicity(&firstParticle->coordinates, &secondParticle->coordinates, DIMENSIONS - 1);
    }

    void RectangleGeometry::EnsureBoundaries(const DomainParticle& initialParticle, DomainParticle* movedParticle, FLOAT_TYPE minNormalizedDistance) const
    {
        throw NotImplementedException("Check code");

//        while(std::abs(movedParticle->coordinates[Axis::X] - packingXSizeHalf) > (packingXSizeHalf - movedParticle->diameter * 0.5))
//            movedParticle->coordinates[Axis::X] = initialParticle->coordinates[Axis::X] + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->diameter;
//
//        while(std::abs(movedParticle->coordinates[Axis::Y] - packingYSizeHalf) > (packingYSizeHalf - movedParticle->diameter * 0.5))
//            movedParticle->coordinates[Axis::Y] = initialParticle->coordinates[Axis::Y] + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->diameter;
//
//        EnsureVerticalPeriodicityAfterRepulsion(movedParticle->coordinates[Axis::Z]);
    }

    bool RectangleGeometry::IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const
    {
        throw NotImplementedException("Implement");
    }
}

