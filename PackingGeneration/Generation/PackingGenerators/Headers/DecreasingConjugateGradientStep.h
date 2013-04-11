// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_DecreasingConjugateGradientStep_h
#define Generation_PackingGenerators_Headers_DecreasingConjugateGradientStep_h

#ifdef GSL_AVAILABLE

#include "BaseConjugateGradientStep.h"

namespace PackingGenerators
{
    // Implements a conjugate gradient optimization step, similar to FBA algorithm, but we minimize elastic intersection energy on each step, instead of moving the particles along their forces.
    class DecreasingConjugateGradientStep : public BaseConjugateGradientStep
    {
    private:
        Core::FLOAT_TYPE initialOuterDiameterRatio;
        static const Core::FLOAT_TYPE NOMINAL_DENSITY_RATIO;

    public:
        DecreasingConjugateGradientStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::MathService* mathService,
                PackingServices::IPairPotential* pairPotential,
                PackingServices::IEnergyService* energyService);

        void DisplaceParticles();

    protected:
        void ResetOuterDiameterRatio();

    private:
        void UpdateOuterDiameterRatio();

        DISALLOW_COPY_AND_ASSIGN(DecreasingConjugateGradientStep);
    };
}

#endif // GSL

#endif /* Generation_PackingGenerators_Headers_DecreasingConjugateGradientStep_h */
