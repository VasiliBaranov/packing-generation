// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_IPairPotential_h
#define Generation_PackingServices_EnergyServices_Headers_IPairPotential_h

#include "Generation/Model/Headers/Types.h"

namespace PackingServices
{
    class IPairPotential
    {
    public:
        virtual Core::Nullable<Core::FLOAT_TYPE> GetEnergy(Core::FLOAT_TYPE firstParticleDiameter, Core::FLOAT_TYPE secondParticleDiameter, Core::FLOAT_TYPE distance) const = 0;

        virtual Core::Nullable<Core::FLOAT_TYPE> GetRepulsionForceFromNeighbor(Core::FLOAT_TYPE particleDiameter, Core::FLOAT_TYPE neighborDiameter, Core::FLOAT_TYPE distance) const = 0;

        virtual ~IPairPotential(){ };
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_IPairPotential_h */

