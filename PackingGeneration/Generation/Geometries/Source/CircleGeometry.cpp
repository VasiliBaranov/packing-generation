// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CircleGeometry.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    CircleGeometry::CircleGeometry(const SystemConfig& config) : BaseGeometry(config)
    {
        if (packingHalfSize[Axis::X] != packingHalfSize[Axis::Y])
        {
            throw NotImplementedException("The system is an ellipse in cross-section. Currently only true circles are supported.");
        }

        VectorUtilities::InitializeWith(&shift, 0.0);
    }

    CircleGeometry::CircleGeometry(const SystemConfig& config, const SpatialVector& shift) : BaseGeometry(config)
    {
        if (packingHalfSize[Axis::X] != packingHalfSize[Axis::Y])
        {
            throw NotImplementedException("The system is an ellipse in cross-section. Currently only true circles are supported.");
        }

        this->shift = shift;
    }

    CircleGeometry::~CircleGeometry()
    {

    }

    FLOAT_TYPE CircleGeometry::GetTotalVolume() const
    {
        FLOAT_TYPE crossSectionSurface = PI * packingHalfSize[Axis::X] * packingHalfSize[Axis::Y];
        return crossSectionSurface * config->packingSize[Axis::Z];
    }

    void CircleGeometry::EnsurePeriodicConditions(DomainParticle* firstParticle, DomainParticle* secondParticle) const
    {
        EnsureVerticalPeriodicity(&firstParticle->coordinates, &secondParticle->coordinates, DIMENSIONS - 1);
    }

    void CircleGeometry::EnsureBoundaries(const DomainParticle& initialParticle, DomainParticle* movedParticle, FLOAT_TYPE minNormalizedDistance) const
    {
        while (~IsSphereInside(movedParticle->coordinates, movedParticle->diameter * 0.5))
        {
            movedParticle->coordinates[Axis::X] = initialParticle.coordinates[Axis::X] + (1.0 - minNormalizedDistance) * (Math::GetNextRandom() - 0.5) * movedParticle->diameter;
            movedParticle->coordinates[Axis::Y] = initialParticle.coordinates[Axis::Y] + (1.0 - minNormalizedDistance) * (Math::GetNextRandom() - 0.5) * movedParticle->diameter;
        }

        EnsureVerticalPeriodicityAfterRepulsion(&movedParticle->coordinates, Axis::Z);
    }

    bool CircleGeometry::IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const
    {
        SpatialVector localCoordinates;
        VectorUtilities::Subtract(sphereCenter, shift, &localCoordinates);
        FLOAT_TYPE distanceToAxisX = localCoordinates[Axis::X] - packingHalfSize[Axis::X];
        FLOAT_TYPE distanceToAxisY = localCoordinates[Axis::Y] - packingHalfSize[Axis::Y];
        FLOAT_TYPE ratio = config->packingSize[Axis::X] / config->packingSize[Axis::Y];

        FLOAT_TYPE distanceToAxisSquare = distanceToAxisX * distanceToAxisX + ratio * ratio * distanceToAxisY * distanceToAxisY;

        // This is probably incorrect in case of an ellipse. maxDistanceToAxisSquare depends on sphereCenter in case of an ellipse (but i scale the distanceToAxisSquare).
        FLOAT_TYPE maxDistanceToAxisSquare = (packingHalfSize[Axis::X] - sphereRadius) * (packingHalfSize[Axis::Y] - sphereRadius);

        bool isInsideCrossSection = distanceToAxisSquare <= maxDistanceToAxisSquare;
        bool isInsideAlongZ = localCoordinates[Axis::Z] >= 0 && localCoordinates[Axis::Z] <= config->packingSize[Axis::Z];
        return isInsideCrossSection && isInsideAlongZ;
    }
}

