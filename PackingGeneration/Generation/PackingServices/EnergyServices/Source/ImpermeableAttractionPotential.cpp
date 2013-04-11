// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ImpermeableAttractionPotential.h"
#include "../Headers/HarmonicPotential.h"

using namespace Core;

namespace PackingServices
{
    // 1e6 is too small, as inner diameter decrease in 1e-6 becomes profitable for energy minimization, even multiplied by 1e6
    const FLOAT_TYPE ImpermeableAttractionPotential::impermeableNormalizer = 1e10;

    ImpermeableAttractionPotential::ImpermeableAttractionPotential(IPairPotential* repulsionPotential)
    {
        this->repulsionPotential = repulsionPotential;
    }

    ImpermeableAttractionPotential::~ImpermeableAttractionPotential()
    {
    }

    Nullable<FLOAT_TYPE> ImpermeableAttractionPotential::GetEnergy(FLOAT_TYPE firstParticleDiameter, FLOAT_TYPE secondParticleDiameter, FLOAT_TYPE distance) const
    {
        Nullable<FLOAT_TYPE> result = repulsionPotential->GetEnergy(firstParticleDiameter, secondParticleDiameter, distance);
        // Intersection exists
        if (result.hasValue)
        {
            result.value *= impermeableNormalizer;
            return result;
        }

        FLOAT_TYPE halfRadius = (firstParticleDiameter + secondParticleDiameter) * 0.5;

        // When the force = distance - halfRadius. In HarmonicPotential force = 1 - distance / halfRadius = (halfRadius - distance) / halfRadius.
        // We would like to make attraction to large particles equal to that to small particles, as they should contribute equally to coordination numbers.
        result.value = (distance - halfRadius) * (distance - halfRadius);
        result.hasValue = true;
        return result;
    }

    Nullable<FLOAT_TYPE> ImpermeableAttractionPotential::GetRepulsionForceFromNeighbor(FLOAT_TYPE particleDiameter, FLOAT_TYPE neighborDiameter, FLOAT_TYPE distance) const
    {
        Nullable<FLOAT_TYPE> result = repulsionPotential->GetRepulsionForceFromNeighbor(particleDiameter, neighborDiameter, distance);
        // Intersection exists
        if (result.hasValue)
        {
            result.value *= impermeableNormalizer;
            return result;
        }

        FLOAT_TYPE halfRadius = (particleDiameter + neighborDiameter) * 0.5;

        // When the force = distance - halfRadius. In HarmonicPotential force = 1 - distance / halfRadius = (halfRadius - distance) / halfRadius.
        // We would like to make attraction to large particles equal to that to small particles, as they should contribute equally to coordination numbers.
        // As far as we return repulsion force, it should be negative when distance > halfRadius.
        result.value = halfRadius - distance;
        result.hasValue = true;
        return result;
    }
}
