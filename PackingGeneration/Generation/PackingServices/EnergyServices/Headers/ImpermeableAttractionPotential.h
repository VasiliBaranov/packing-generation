// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_EnergyServices_Headers_ImpermeableAttractionPotential_h
#define Generation_PackingServices_EnergyServices_Headers_ImpermeableAttractionPotential_h

#include "Core/Headers/Macros.h"
#include "IPairPotential.h"

namespace PackingServices
{
    class ImpermeableAttractionPotential : public virtual IPairPotential
    {
    private:
        const static Core::FLOAT_TYPE impermeableNormalizer;
        IPairPotential* repulsionPotential;

    public:
        explicit ImpermeableAttractionPotential(IPairPotential* repulsionPotential);

        ~ImpermeableAttractionPotential();

        OVERRIDE Core::Nullable<Core::FLOAT_TYPE> GetEnergy(Core::FLOAT_TYPE firstParticleDiameter, Core::FLOAT_TYPE secondParticleDiameter, Core::FLOAT_TYPE distance) const;

        OVERRIDE Core::Nullable<Core::FLOAT_TYPE> GetRepulsionForceFromNeighbor(Core::FLOAT_TYPE particleDiameter, Core::FLOAT_TYPE neighborDiameter, Core::FLOAT_TYPE distance) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(ImpermeableAttractionPotential);
    };
}

#endif /* Generation_PackingServices_EnergyServices_Headers_ImpermeableAttractionPotential_h */

