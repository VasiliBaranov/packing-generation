// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/CircleGeometry.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/Constants.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    CircleGeometry::CircleGeometry(const SystemConfig& config) : BaseGeometry(config)
    {
        throw NotImplementedException("Check code");
    }

    CircleGeometry::~CircleGeometry()
    {

    }

    FLOAT_TYPE CircleGeometry::GetTotalVolume() const
    {
        return packingHalfSize[Axis::X] * packingHalfSize[Axis::Y] * config->packingSize[Axis::Z] * PI;
    }

    void CircleGeometry::EnsurePeriodicConditions(DomainParticle* firstParticle, DomainParticle* secondParticle) const
    {
        EnsureVerticalPeriodicity(&firstParticle->coordinates, &secondParticle->coordinates, DIMENSIONS - 1);
    }

    void CircleGeometry::EnsureBoundaries(const DomainParticle& initialParticle, DomainParticle* movedParticle, FLOAT_TYPE minNormalizedDistance) const
    {
        throw NotImplementedException("Check code");

//        Position center;
//
//        movedParticle->coordinates
//
//        FLOAT_TYPE distanceToBoundarySquare =
//                (movedParticle->x - packingXSizeHalf) * (movedParticle->x - packingXSizeHalf) +
//                (config->packingXSize * config->packingXSize) / (config->packingYSize * config->packingYSize) * (movedParticle->y - packingYSizeHalf) * (movedParticle->y - packingYSizeHalf);
//
//        FLOAT_TYPE maxDistanceToBoundarySquare = (config->packingXSize - movedParticle->d) * (config->packingXSize - movedParticle->d) * 0.25;
//
//        while(distanceToBoundarySquare > maxDistanceToBoundarySquare)
//        {
//            movedParticle->x = initialParticle->x + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->d;
//            movedParticle->y = initialParticle->y + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->d;
//        }
//
//        EnsureVerticalPeriodicityAfterRepulsion(movedParticle->z);
    }


}

