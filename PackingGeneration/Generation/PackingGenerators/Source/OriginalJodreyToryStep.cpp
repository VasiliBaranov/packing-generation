// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/OriginalJodreyToryStep.h"
#include <stdio.h>
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/PackingServices/DistanceServices/Headers/IClosestPairProvider.h"
#include "Generation/Geometries/Headers/IGeometry.h"
#include "Generation/Model/Headers/Config.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    OriginalJodreyToryStep::OriginalJodreyToryStep(GeometryService* geometryService,
            IClosestPairProvider* closestPairProvider,
            MathService* mathService) :
            BasePackingStep(geometryService, NULL, closestPairProvider, mathService)
    {
        isOuterDiameterChanging = true;
        canOvercomeTheoreticalDensity = true;
    }

    OriginalJodreyToryStep::~OriginalJodreyToryStep()
    {

    }

    void OriginalJodreyToryStep::SetParticles(Packing* particles)
    {
        BasePackingStep::SetParticles(particles);

        closestPair = closestPairProvider->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        ResetOuterDiameterRatio();
    }

    void OriginalJodreyToryStep::DisplaceParticles()
    {
        Packing& particlesRef = *particles;
        DomainParticle* firstParticle = &particlesRef[closestPair.firstParticleIndex];
        DomainParticle* secondParticle = &particlesRef[closestPair.secondParticleIndex];

        RepulseClosestPair(firstParticle, secondParticle);

        closestPair = closestPairProvider->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        UpdateOuterDiameterRatio();
    }

    void OriginalJodreyToryStep::RepulseClosestPair(DomainParticle* firstParticle, DomainParticle* secondParticle)
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

    void OriginalJodreyToryStep::RepulseParticle(const DomainParticle& particleOriginal, const DomainParticle& particlePeriodic, DomainParticle* particle, const DomainParticle& secondParticlePeriodic)
    {
        // Repulse to preserve a center of mass of a pair to handle polydispersivity in a natural way
        FLOAT_TYPE firstMass = particlePeriodic.diameter * particlePeriodic.diameter * particlePeriodic.diameter;
        FLOAT_TYPE secondMass = secondParticlePeriodic.diameter * secondParticlePeriodic.diameter * secondParticlePeriodic.diameter;

        FLOAT_TYPE repulsionFactor = secondMass / (firstMass + secondMass) * (outerDiameterRatio - innerDiameterRatio) / innerDiameterRatio;

        closestPairProvider->StartMove(particle->index);

        SpatialVector displacement;
        VectorUtilities::Subtract(particlePeriodic.coordinates, secondParticlePeriodic.coordinates, &displacement);
        VectorUtilities::MultiplyByValue(displacement, repulsionFactor, &displacement);
        VectorUtilities::Add(particlePeriodic.coordinates, displacement, &particle->coordinates);

        // TODO: check if the last parameter is correct for non-bulk geometries; may be we should pass a ratio, recalculated after repulsion.
        geometry->EnsureBoundaries(particleOriginal, particle, innerDiameterRatio);

        closestPairProvider->EndMove();
    }

    void OriginalJodreyToryStep::ResetGeneration()
    {
        ResetDistanceProvider();
        ResetOuterDiameterRatio();
    }

    void OriginalJodreyToryStep::UpdateOuterDiameterRatio()
    {
        FLOAT_TYPE nominalPorosity = CalculateCurrentPorosity(outerDiameterRatio);
        FLOAT_TYPE actualPorosity = CalculateCurrentPorosity(innerDiameterRatio);

        FLOAT_TYPE packingFractionDifference = actualPorosity - nominalPorosity; // it's equal to the nominalPackingFraction - actualPackingFraction
        if (packingFractionDifference < 0)
        {
            return;
        }

        int j = static_cast<int>(ceil(- log10(packingFractionDifference))); //variable from the original article
        outerDiameterRatio -= pow(0.5, j) * generationConfig->contractionRate / config->particlesCount;
    }

    void OriginalJodreyToryStep::ResetDistanceProvider()
    {
        printf("Resetting distance provider...\n");

        FLOAT_TYPE oldInnerDiameterRatio = innerDiameterRatio;

        closestPairProvider->SetParticles(*particles);

        ParticlePair closestPair = closestPairProvider->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        if (std::abs(oldInnerDiameterRatio - innerDiameterRatio) > 1e-6)
        {
            throw InvalidOperationException("InnerDiameterRatio after reset is different from the previous value. Bugs in distanceProvider.");
        }
    }

    void OriginalJodreyToryStep::ResetOuterDiameterRatio()
    {
        outerDiameterRatio = pow(totalVolume / particlesVolume, 1.0 / DIMENSIONS);
    }
}

