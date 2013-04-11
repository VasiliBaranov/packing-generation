// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BaseGeometry.h"
#include <cmath>
#include <stdlib.h>
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Constants.h"
#include "Core/Headers/Exceptions.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    BaseGeometry::BaseGeometry(const SystemConfig& config)
    {
        this->config = &config;
        VectorUtilities::MultiplyByValue(config.packingSize, 0.5, &packingHalfSize);
    }

    BaseGeometry::~BaseGeometry()
    {

    }

    void BaseGeometry::EnsureVerticalPeriodicity(Core::SpatialVector* x, Core::SpatialVector* y, int dimension) const
    {
        Core::SpatialVector& xRef = *x;
        Core::SpatialVector& yRef = *y;

        if (std::abs(xRef[dimension] - yRef[dimension]) > packingHalfSize[dimension])
        {
            if (xRef[dimension] > yRef[dimension])
            {
                yRef[dimension] += config->packingSize[dimension];
            }
            else
            {
                xRef[dimension] += config->packingSize[dimension];
            }
        }
    }

    void BaseGeometry::EnsureVerticalPeriodicityAfterRepulsion(Core::SpatialVector* x, int dimension) const
    {
        Core::SpatialVector& xRef = *x;

        while (xRef[dimension] < 0.0)
        {
            xRef[dimension] += config->packingSize[dimension];
        }
        while (xRef[dimension] >= config->packingSize[dimension])
        {
            xRef[dimension] -= config->packingSize[dimension];
        }
    }
}

