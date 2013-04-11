// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BezrukovPotential.h"
#include "Core/Headers/Exceptions.h"

using namespace Core;

namespace PackingServices
{
    BezrukovPotential::BezrukovPotential()
    {
    }

    Nullable<FLOAT_TYPE> BezrukovPotential::GetEnergy(FLOAT_TYPE firstParticleDiameter, FLOAT_TYPE secondParticleDiameter, FLOAT_TYPE distance) const
    {
        throw NotImplementedException("BezrukovPotential::GetEnergy not implemented.");
    }

    Nullable<FLOAT_TYPE> BezrukovPotential::GetRepulsionForceFromNeighbor(FLOAT_TYPE particleDiameter, FLOAT_TYPE neighborDiameter, FLOAT_TYPE distance) const
    {
        Nullable<FLOAT_TYPE> result;
        FLOAT_TYPE halfDiameter = (particleDiameter + neighborDiameter) * 0.5;
        FLOAT_TYPE diameterRatio = distance / halfDiameter;
        if (diameterRatio >= 1.0)
        {
            result.hasValue = false;
            return result;
        }

        // It's the repulsion force from neighbor, so it should be positive in case of intersecting elastic particles
        FLOAT_TYPE force = particleDiameter * neighborDiameter * (1.0 - distance * distance / halfDiameter / halfDiameter);
        result.hasValue = true;
        result.value = force;
        return result;
    }
}

