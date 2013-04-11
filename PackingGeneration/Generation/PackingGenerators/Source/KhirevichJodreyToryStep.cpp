// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/KhirevichJodreyToryStep.h"
#include <stdio.h>
#include "Generation/PackingServices/DistanceServices/Headers/IClosestPairProvider.h"
#include "Generation/Geometries/Headers/IGeometry.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Exceptions.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    KhirevichJodreyToryStep::KhirevichJodreyToryStep(GeometryService* geometryService,
            IClosestPairProvider* closestPairProvider,
            MathService* mathService) :
            BasePackingStep(geometryService, NULL, closestPairProvider, mathService)
    {
        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = false;
    }

    KhirevichJodreyToryStep::~KhirevichJodreyToryStep()
    {

    }

    void KhirevichJodreyToryStep::SetParticles(Packing* particles)
    {
        BasePackingStep::SetParticles(particles);

        ResetClosestParticleParams();
    }

    void KhirevichJodreyToryStep::DisplaceParticles()
    {
        Packing& particlesRef = *particles;
        DomainParticle* firstParticle = &particlesRef[closestPair.firstParticleIndex];
        DomainParticle* secondParticle = &particlesRef[closestPair.secondParticleIndex];

        RepulseClosestPair(firstParticle, secondParticle);

        closestPair = closestPairProvider->FindClosestPair();

        if (innerDiameterRatioSquare < closestPair.normalizedDistanceSquare)
        {
            ResetClosestParticleParams(closestPair.normalizedDistanceSquare);
        }
    }

    void KhirevichJodreyToryStep::RepulseClosestPair(DomainParticle* firstParticle, DomainParticle* secondParticle)
    {
        // These particle copies are needed to ensure boundaries in non-bulk geometries
        DomainParticle firstParticleOriginal = *firstParticle;
        DomainParticle secondParticleOriginal = *secondParticle;

        // These particle copies are needed to apply periodic boundaries and to move particles into the closest possible position
        DomainParticle firstParticlePeriodic = *firstParticle;
        DomainParticle secondParticlePeriodic = *secondParticle;
        geometry->EnsurePeriodicConditions(&firstParticlePeriodic, &secondParticlePeriodic);

        RepulseParticle(firstParticleOriginal, firstParticlePeriodic, firstParticle, secondParticlePeriodic);
        RepulseParticle(secondParticleOriginal, secondParticlePeriodic, secondParticle, firstParticlePeriodic);
    }

    void KhirevichJodreyToryStep::RepulseParticle(const DomainParticle& particleOriginal, const DomainParticle& particlePeriodic, DomainParticle* particle, const DomainParticle& secondParticlePeriodic)
    {
        closestPairProvider->StartMove(particle->index);

        SpatialVector left;
        SpatialVector right;

        VectorUtilities::MultiplyByValue(particlePeriodic.coordinates, (repulsionFactor + 1.0) / 2.0, &left);
        VectorUtilities::MultiplyByValue(secondParticlePeriodic.coordinates, (repulsionFactor - 1.0) / 2.0, &right);
        VectorUtilities::Subtract(left, right, &particle->coordinates);

        //TODO: check if the last parameter is correct for non-bulk geometries; may be we should pass a ratio, recalculated after repulsion.
        geometry->EnsureBoundaries(particleOriginal, particle, innerDiameterRatio);

        closestPairProvider->EndMove();
    }

    void KhirevichJodreyToryStep::ResetGeneration()
    {
        ResetDistanceProvider();
    }

    void KhirevichJodreyToryStep::ResetDistanceProvider()
    {
        printf("Resetting distance provider...\n");

        FLOAT_TYPE oldInnerDiameterRatio = innerDiameterRatio;

        closestPairProvider->SetParticles(*particles);
        ResetClosestParticleParams();

        if (std::abs(oldInnerDiameterRatio - innerDiameterRatio) > 1e-6)
        {
            throw InvalidOperationException("InnerDiameterRatio after reset is different from the previous value. Bugs in distanceProvider.");
        }
    }

    void KhirevichJodreyToryStep::ResetClosestParticleParams()
    {
        closestPair = closestPairProvider->FindClosestPair();
        ResetClosestParticleParams(closestPair.normalizedDistanceSquare);
    }

    void KhirevichJodreyToryStep::ResetClosestParticleParams(FLOAT_TYPE closestNormalizedDistanceSquare)
    {
        innerDiameterRatioSquare = closestNormalizedDistanceSquare;
        innerDiameterRatio = sqrt(innerDiameterRatioSquare);
        repulsionFactor = (1.0 + log10(1. / innerDiameterRatio)) / innerDiameterRatio;
    }
}

