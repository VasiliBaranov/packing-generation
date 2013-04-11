// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_DensificationStep_h
#define Generation_PackingGenerators_Headers_DensificationStep_h

#ifdef GSL_AVAILABLE

#include "BaseConjugateGradientStep.h"

namespace PackingGenerators
{
    class DensificationStep : public BaseConjugateGradientStep
    {
    private:
        bool shouldContinue;
        const Model::ModellingContext* context;

        static const Core::FLOAT_TYPE localGradientTolerance;
        static const Core::FLOAT_TYPE globalGradientTolerance;

    public:
        DensificationStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::MathService* mathService,
                PackingServices::IPairPotential* pairPotential,
                PackingServices::IEnergyService* energyService);

        void SetContext(const Model::ModellingContext& context);

        void SetParticles(Model::Packing* particles);

        void DisplaceParticles();

        bool ShouldContinue() const;

    protected:
        void ResetOuterDiameterRatio();

    private:
        Core::FLOAT_TYPE GetEnergyPerParticle() const;

        void UpdateParticleRadii();

        DISALLOW_COPY_AND_ASSIGN(DensificationStep);
    };
}

#endif // GSL

#endif /* Generation_PackingGenerators_Headers_DensificationStep_h */
