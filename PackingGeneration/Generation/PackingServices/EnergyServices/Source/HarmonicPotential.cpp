// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/HarmonicPotential.h"
#include <cmath>
using namespace Core;

namespace PackingServices
{
    HarmonicPotential::HarmonicPotential(FLOAT_TYPE energyPower)
    {
        this->energyPower = energyPower;
    }

    Nullable<FLOAT_TYPE> HarmonicPotential::GetEnergy(FLOAT_TYPE firstParticleDiameter, FLOAT_TYPE secondParticleDiameter, FLOAT_TYPE distance) const
    {
        Nullable<FLOAT_TYPE> result;
        FLOAT_TYPE diameterRatio = 2.0 * distance / (firstParticleDiameter + secondParticleDiameter);
        if (diameterRatio >= 1.0)
        {
            result.hasValue = false;
            return result;
        }

        // From Chaudhury, Berthier, Sastry (2010).
        // If the potential energy is potentialNormalizer * (1.0 - distance / halfDiameter) ^ energyPower.
        FLOAT_TYPE contractionEnergy;
        if (energyPower == 2.0)
        {
            contractionEnergy = (1.0 - diameterRatio) * (1.0 - diameterRatio);
        }
        else
        {
            contractionEnergy = pow(1.0 - diameterRatio, energyPower);
        }

        result.hasValue = true;
        result.value = contractionEnergy;
        return result;
    }

    Nullable<FLOAT_TYPE> HarmonicPotential::GetRepulsionForceFromNeighbor(FLOAT_TYPE particleDiameter, FLOAT_TYPE neighborDiameter, FLOAT_TYPE distance) const
    {
        Nullable<FLOAT_TYPE> result;
        FLOAT_TYPE halfDiameter = (particleDiameter + neighborDiameter) * 0.5;
        FLOAT_TYPE diameterRatio = distance / halfDiameter;
        if (diameterRatio >= 1.0)
        {
            result.hasValue = false;
            return result;
        }

        // If the potential energy is potentialNormalizer * (1.0 - distance / halfDiameter) ^ energyPower.
        FLOAT_TYPE force;
        if (energyPower == 2.0)
        {
            force = 1.0 - diameterRatio;
        }
        else
        {
            force = pow(1.0 - diameterRatio, energyPower - 1.0);
        }
        force = energyPower * force / halfDiameter;

        result.hasValue = true;
        result.value = force;
        return result;
    }
}
