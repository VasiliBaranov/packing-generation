// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BezrukovJodreyToryStep.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/EnergyServices/Headers/IEnergyService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/Geometries/Headers/IGeometry.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    const FLOAT_TYPE BezrukovJodreyToryStep::NOMINAL_DENSITY_RATIO = 1.2;
    const FLOAT_TYPE BezrukovJodreyToryStep::FORCE_SCALING_FACTOR = 0.5;

    BezrukovJodreyToryStep::BezrukovJodreyToryStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IEnergyService* energyService,
            IPairPotential* pairPotential) :
            BasePackingStep(geometryService, neighborProvider, NULL, mathService)
    {
        this->pairPotential = pairPotential;
        this->energyService = energyService;

        isOuterDiameterChanging = true;
        canOvercomeTheoreticalDensity = true;
    }

    BezrukovJodreyToryStep::~BezrukovJodreyToryStep()
    {
    }

    void BezrukovJodreyToryStep::SetParticles(Packing* particles)
    {
        energyService->SetContext(*context); // just not to overload base SetContext :-)

        BasePackingStep::SetParticles(particles);
        energyService->SetParticles(*particles);

        particleForces.resize(config->particlesCount); // Config may have changed since previous generation

        ResetOuterDiameterRatio(); // outer diameter ratio will be needed in FillAllParticleDisplacements

        // To set up innerDiameterRatio and to prepare for the first displacement step
        ParticlePair closestPair = energyService->FillParticleForces(1.0 / outerDiameterRatio, *pairPotential, &particleForces);
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
    }

    void BezrukovJodreyToryStep::DisplaceParticles()
    {
        RepulseAllParticles();

        ParticlePair closestPair = energyService->FillParticleForces(1.0 / outerDiameterRatio, *pairPotential, &particleForces);
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        UpdateOuterDiameterRatio();
    }

    void BezrukovJodreyToryStep::ResetGeneration()
    {
        ResetOuterDiameterRatio();
    }

    void BezrukovJodreyToryStep::UpdateOuterDiameterRatio()
    {
        FLOAT_TYPE innerPorosity = CalculateCurrentPorosity(innerDiameterRatio);

        // Formula from the Bezrukov, Bargiel, Stoyan 2002
        FLOAT_TYPE nominalDensity = (1.0 - theoreticalPorosity) * NOMINAL_DENSITY_RATIO;
        FLOAT_TYPE nominalPorosity = 1.0 - nominalDensity;
        FLOAT_TYPE packingFractionDifference = innerPorosity - nominalPorosity;
        if (packingFractionDifference < 0.0)
        {
            return;
        }

        double j = ceil(-log10(packingFractionDifference));
        outerDiameterRatio -= pow(0.5, j) * initialOuterDiameterRatio * generationConfig->contractionRate;
    }

    void BezrukovJodreyToryStep::RepulseAllParticles()
    {
        Packing& particlesRef = *particles;

        SpatialVector displacement;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DomainParticle& particle = particlesRef[particleIndex];
            neighborProvider->StartMove(particle.index);

            // Need to multiply twice by outerDiameterRatio, as energy service computes forces by contracting distance, not expanding diameters, so Bezrukov formula is computed a little incorrectly.
            VectorUtilities::MultiplyByValue(particleForces[particleIndex], FORCE_SCALING_FACTOR / 2.0 / particle.diameter * outerDiameterRatio * outerDiameterRatio, &displacement);
            VectorUtilities::Add(particle.coordinates, displacement, &particle.coordinates);
            geometry->EnsureBoundaries(particle, &particle, innerDiameterRatio);

            neighborProvider->EndMove();
        }
    }

    void BezrukovJodreyToryStep::ResetOuterDiameterRatio()
    {
        outerDiameterRatio = pow(NOMINAL_DENSITY_RATIO, 1.0 / 3.0);
        initialOuterDiameterRatio = outerDiameterRatio;
    }
}

