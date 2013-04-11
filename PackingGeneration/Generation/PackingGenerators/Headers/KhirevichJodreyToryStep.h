// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_KhirevichJodreyToryStep_h
#define Generation_PackingGenerators_Headers_KhirevichJodreyToryStep_h

#include "BasePackingStep.h"

namespace PackingGenerators
{
    // Implements a modified Jodrey-Tory algorithm, uncapable of overcoming RCP limits. See Khirevich dissertation.
    class KhirevichJodreyToryStep : public BasePackingStep
    {
    private:
        // Working variables
        Core::FLOAT_TYPE repulsionFactor, innerDiameterRatioSquare;
        Model::ParticlePair closestPair;

    public:
        KhirevichJodreyToryStep(PackingServices::GeometryService* geometryService,
                PackingServices::IClosestPairProvider* closestPairProvider,
                PackingServices::MathService* mathService);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        ~KhirevichJodreyToryStep();

    private:
        void RepulseClosestPair(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle);

        void RepulseParticle(const Model::DomainParticle& particleOriginal, const Model::DomainParticle& particlePeriodic, Model::DomainParticle* particle, const Model::DomainParticle& secondParticlePeriodic);

        void ResetDistanceProvider();

        void ResetClosestParticleParams();

        void ResetClosestParticleParams(Core::FLOAT_TYPE closestNormalizedDistanceSquare);

        DISALLOW_COPY_AND_ASSIGN(KhirevichJodreyToryStep);
    };
}

#endif /* Generation_PackingGenerators_Headers_KhirevichJodreyToryStep_h */
