// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_SimpleConjugateGradientStep_h
#define Generation_PackingGenerators_Headers_SimpleConjugateGradientStep_h

#ifdef GSL_AVAILABLE

#include "BaseConjugateGradientStep.h"

namespace PackingGenerators
{
    // Implements a conjugate gradient optimization step, similar to FBA. But instead of shifting particles along their forces find optimium shifts for each particle (which is a single optimization step).
    class SimpleConjugateGradientStep : public BaseConjugateGradientStep
    {
    private:
        bool shouldContinue;

    public:
        SimpleConjugateGradientStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::MathService* mathService,
                PackingServices::IPairPotential* pairPotential,
                PackingServices::IEnergyService* energyService);

        void SetParticles(Model::Packing* particles);

        void DisplaceParticles();

        bool ShouldContinue() const;

    protected:
        void ResetOuterDiameterRatio();

    private:
        DISALLOW_COPY_AND_ASSIGN(SimpleConjugateGradientStep);
    };
}

#endif // GSL

#endif /* Generation_PackingGenerators_Headers_SimpleConjugateGradientStep_h */
