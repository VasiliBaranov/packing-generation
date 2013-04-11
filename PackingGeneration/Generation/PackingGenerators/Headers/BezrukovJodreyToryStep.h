// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_BezrukovJodreyToryStep_h
#define Generation_PackingGenerators_Headers_BezrukovJodreyToryStep_h

#include "BasePackingStep.h"
namespace PackingServices { class IEnergyService; }
namespace PackingServices { class IPairPotential; }

namespace PackingGenerators
{
    // Implements a modified Jodrey-Tory algorithm, see Bezrukov, Bargiel, Stoyan (2002) and Jodrey, Tory (1985) "Computer simulation of close random packing of equal spheres".
    class BezrukovJodreyToryStep : public BasePackingStep
    {
    private:
        // Working variables
        Core::FLOAT_TYPE initialOuterDiameterRatio;
        std::vector<Core::SpatialVector> particleForces;

        PackingServices::IEnergyService* energyService;
        PackingServices::IPairPotential* pairPotential;

        static const Core::FLOAT_TYPE NOMINAL_DENSITY_RATIO;
        static const Core::FLOAT_TYPE FORCE_SCALING_FACTOR;

    public:
        BezrukovJodreyToryStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::MathService* mathService,
                PackingServices::IEnergyService* energyService,
                PackingServices::IPairPotential* pairPotential);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        ~BezrukovJodreyToryStep();

    private:
        void RepulseAllParticles();

        void ResetOuterDiameterRatio();

        void UpdateOuterDiameterRatio();

        DISALLOW_COPY_AND_ASSIGN(BezrukovJodreyToryStep);
    };
}

#endif /* Generation_PackingGenerators_Headers_BezrukovJodreyToryStep_h */
