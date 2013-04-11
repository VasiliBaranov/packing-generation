// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_IncreasingConjugateGradientStep_h
#define Generation_PackingGenerators_Headers_IncreasingConjugateGradientStep_h

#ifdef GSL_AVAILABLE

#include "BaseConjugateGradientStep.h"

namespace PackingGenerators
{
    // Implements a conjugate gradient optimization step, as explained in
    // Xu et al (2005) Random close packing revisited: Ways to pack frictionless disks.
    // Gao, Blawdziewicz, O'Hern (2006) Frequency distribution of mechanically stable disk packings.
    class IncreasingConjugateGradientStep : public BaseConjugateGradientStep
    {
    private:
        bool shouldContinue;
        bool previousOuterDiameterDirectionIsUp;
        Core::FLOAT_TYPE currentContractionRate;

        static const Core::FLOAT_TYPE relativeEnergyTolerance;
        static const Core::FLOAT_TYPE minEnergyPerParticle;

    public:
        IncreasingConjugateGradientStep(PackingServices::GeometryService* geometryService,
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
        void UpdateOuterDiameterRatio();

        Core::FLOAT_TYPE GetEnergyPerParticle() const;

        DISALLOW_COPY_AND_ASSIGN(IncreasingConjugateGradientStep);
    };
}

#endif // GSL

#endif /* Generation_PackingGenerators_Headers_IncreasingConjugateGradientStep_h */
