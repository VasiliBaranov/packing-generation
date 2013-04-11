// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/TrapezoidGeometry.h"
#include <stdio.h>
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/Constants.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    TrapezoidGeometry::TrapezoidGeometry(const SystemConfig& config) : BaseGeometry(config)
    {
        throw NotImplementedException("Check code");

        icos_alpha = 1.0/cos(config.alpha*PI/180.0);
        a_trapezoid = config.packingSize[Axis::Y] * tan(config.alpha * PI / 180.0);
        a_div_y_trapezoid = a_trapezoid / config.packingSize[Axis::Y];
//        config->packingSize[Axis::X] += a_trapezoid;
        printf("icos %lf   a_tr %lf   a_div_y %lf; bottom base is %lf, top base if %lf\n", icos_alpha, a_trapezoid, a_div_y_trapezoid, config.packingSize[Axis::X], config.packingSize[Axis::X] - 2.0 * a_trapezoid);
    }

    TrapezoidGeometry::~TrapezoidGeometry()
    {

    }

    FLOAT_TYPE TrapezoidGeometry::GetTotalVolume() const
    {
        SpatialVector temp = config->packingSize;
        temp[0] -= a_trapezoid;
        return VectorUtilities::GetProduct(temp);
    }

    void TrapezoidGeometry::EnsurePeriodicConditions(DomainParticle* firstParticle, DomainParticle* secondParticle) const
    {
        EnsureVerticalPeriodicity(&firstParticle->coordinates, &secondParticle->coordinates, DIMENSIONS - 1);
    }

    void TrapezoidGeometry::EnsureBoundaries(const DomainParticle& initialParticle, DomainParticle* movedParticle, FLOAT_TYPE minNormalizedDistance) const
    {
        throw NotImplementedException("Check code");

//        FLOAT_TYPE dx=0.0;
//
//        while((std::abs(movedParticle->coordinates[Axis::Y] - packingYSizeHalf) > (packingYSizeHalf - movedParticle->diameter * 0.5)))
//        {
//            movedParticle->coordinates[Axis::Y] = initialParticle->coordinates[Axis::Y] + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->diameter;
//            dx = (movedParticle->coordinates[Axis::Y] - initialParticle->coordinates[Axis::Y]) * a_div_y_trapezoid;
//        }
//
//        x_left_trapezoid = movedParticle->coordinates[Axis::Y] * a_div_y_trapezoid;
//        x_right_trapezoid = config->packingXSize - a_div_y_trapezoid * movedParticle->coordinates[Axis::Y];
//
//        while (movedParticle->coordinates[Axis::X] > (x_right_trapezoid - 0.5 * movedParticle->diameter * icos_alpha))
//        {
//            movedParticle->coordinates[Axis::X] = (initialParticle->coordinates[Axis::X] - dx) + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->diameter;
//        }
//
//        while (movedParticle->coordinates[Axis::X] < (0.5 * movedParticle->diameter * icos_alpha + x_left_trapezoid))
//        {
//            movedParticle->coordinates[Axis::X] = (initialParticle->coordinates[Axis::X] + dx) + (1.0 - minNormalizedDistance) * (drand48() - 0.5) * movedParticle->diameter;
//        }
//
//        EnsureVerticalPeriodicityAfterRepulsion(movedParticle->coordinates[Axis::Z]);
    }
}

