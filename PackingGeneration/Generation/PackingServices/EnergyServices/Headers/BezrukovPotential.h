// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_BezrukovPotential_h
#define Generation_PackingServices_EnergyServices_Headers_BezrukovPotential_h

#include "Core/Headers/Macros.h"
#include "IPairPotential.h"

namespace PackingServices
{
    // See Bezrukov, Bargiel, Stoyan (2002) and Jodrey, Tory (1985) "Computer simulation of close random packing of equal spheres".
    class BezrukovPotential : public virtual IPairPotential
    {
    public:
        BezrukovPotential();

        OVERRIDE Core::Nullable<Core::FLOAT_TYPE> GetEnergy(Core::FLOAT_TYPE firstParticleDiameter, Core::FLOAT_TYPE secondParticleDiameter, Core::FLOAT_TYPE distance) const;

        OVERRIDE Core::Nullable<Core::FLOAT_TYPE> GetRepulsionForceFromNeighbor(Core::FLOAT_TYPE particleDiameter, Core::FLOAT_TYPE neighborDiameter, Core::FLOAT_TYPE distance) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(BezrukovPotential);
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_BezrukovPotential_h */

