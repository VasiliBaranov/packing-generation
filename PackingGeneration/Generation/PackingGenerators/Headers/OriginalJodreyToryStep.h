// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_OriginalJodreyToryStep_h
#define Generation_PackingGenerators_Headers_OriginalJodreyToryStep_h

#include "BasePackingStep.h"
namespace PackingServices { class IClosestPairProvider; }

namespace PackingGenerators
{
    // Implements a Jodrey-Tory algorithm, see Jodrey, Tory (1985) Computer simulation of close random packing of equal spheres.
    class OriginalJodreyToryStep : public BasePackingStep
    {
    private:
        Model::ParticlePair closestPair;

    public:
        OriginalJodreyToryStep(PackingServices::GeometryService* geometryService,
                PackingServices::IClosestPairProvider* closestPairProvider,
                PackingServices::MathService* mathService);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        ~OriginalJodreyToryStep();

    private:
        void RepulseClosestPair(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle);

        void RepulseParticle(const Model::DomainParticle& particleOriginal, const Model::DomainParticle& particlePeriodic, Model::DomainParticle* particle, const Model::DomainParticle& secondParticlePeriodic);

        void ResetOuterDiameterRatio();

        void UpdateOuterDiameterRatio();

        void ResetDistanceProvider();

        DISALLOW_COPY_AND_ASSIGN(OriginalJodreyToryStep);
    };
}

#endif /* Generation_PackingGenerators_Headers_OriginalJodreyToryStep_h */
