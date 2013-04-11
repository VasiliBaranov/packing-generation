// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_MonteCarloStep_h
#define Generation_PackingGenerators_Headers_MonteCarloStep_h

#include "BasePackingStep.h"
namespace PackingServices { class DistanceService; }

namespace PackingGenerators
{
    // Implements a modified Jodrey-Tory algorithm, uncapable of overcoming RCP limits. See Khirevich dissertation.
    class MonteCarloStep : public BasePackingStep
    {
    private:
        // Services
        PackingServices::DistanceService* distanceService;

        // Working variables
        Core::FLOAT_TYPE minNormalizedDistance;
        Core::FLOAT_TYPE maxDisplacementLength;

        Core::FLOAT_TYPE maxDisplacementToFreeLengthRatio;
        Core::FLOAT_TYPE initialExpansionFactor;

        bool isFirstRun;

        static const Core::FLOAT_TYPE EPSILON;

    public:
        MonteCarloStep(PackingServices::GeometryService* geometryService,
                PackingServices::DistanceService* distanceService,
                PackingServices::MathService* mathService);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        ~MonteCarloStep();

    private:
        Model::ParticlePair DisplaceParticlesOneTime(bool* success);

        void DisplaceParticle(Model::DomainParticle* particle);

        void UpdateParameters(Model::ParticlePair closestPair);

        void ArrangeInCubicArray();

        DISALLOW_COPY_AND_ASSIGN(MonteCarloStep);
    };
}

#endif /* Generation_PackingGenerators_Headers_MonteCarloStep_h */
