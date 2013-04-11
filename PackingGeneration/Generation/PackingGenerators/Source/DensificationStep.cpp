// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/DensificationStep.h"

#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"

#ifdef GSL_AVAILABLE

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
    const FLOAT_TYPE DensificationStep::localGradientTolerance = 1e-5;
    const FLOAT_TYPE DensificationStep::globalGradientTolerance = 1e-16;

    DensificationStep::DensificationStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IPairPotential* pairPotential,
            IEnergyService* energyService) :
            BaseConjugateGradientStep(geometryService, neighborProvider, mathService, pairPotential, energyService)
    {
        isOuterDiameterChanging = true;
        canOvercomeTheoreticalDensity = true;
    }

    void DensificationStep::SetContext(const ModellingContext& context)
    {
        BaseConjugateGradientStep::SetContext(context);
        this->context = &context;
    }

    void DensificationStep::SetParticles(Packing* particles)
    {
        BaseConjugateGradientStep::SetParticles(particles);
        shouldContinue = true;
    }

    void DensificationStep::DisplaceParticles()
    {
        InitializeMinimization();

        int result = gsl_multimin_test_gradient(minimizer->gradient, globalGradientTolerance * potentialNormalizer);
        shouldContinue = (result == GSL_CONTINUE);
        if (!shouldContinue)
        {
            printf("Stop algorithm. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, GetEnergyPerParticle());
        }

        while (true)
        {
            int errorStatus = gsl_multimin_fdfminimizer_iterate(minimizer);
            bool localShouldContinue = (errorStatus == GSL_SUCCESS);

            // Iteration is not making progress towards solution.
            if (errorStatus == GSL_ENOPROG)
            {
                printf("Stop iteration. Iteration is not making progress towards solution. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, GetEnergyPerParticle());
                break;
            }

            // We should always use one of the GSL termination conditions, as when stopping just by stationary or zero energy the GSL sometimes hangs.
            if (localShouldContinue)
            {
                int result = gsl_multimin_test_gradient(minimizer->gradient, localGradientTolerance * potentialNormalizer);
                localShouldContinue = (result == GSL_CONTINUE);
            }

            if (!localShouldContinue)
            {
                printf("Stop iteration. Gradient too small. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, GetEnergyPerParticle());
                break;
            }
        }

        UpdateParticleRadii();

        // Need to set particles once again, as have updated particle radii. This is a hack, as this method is not intended to be called from inside this class.
        SetParticles(particles);

        ResetOuterDiameterRatio();
    }

    FLOAT_TYPE DensificationStep::GetEnergyPerParticle() const
    {
        FLOAT_TYPE energyPerParticle = minimizer->f / potentialNormalizer / config->particlesCount;
        return energyPerParticle;
    }

    void DensificationStep::ResetOuterDiameterRatio()
    {
        outerDiameterRatio = innerDiameterRatio;
    }

    bool DensificationStep::ShouldContinue() const
    {
        return shouldContinue;
    }

    void DensificationStep::UpdateParticleRadii()
    {
        FLOAT_TYPE maxDiameterIncrease;
        const FLOAT_TYPE minAllowedDiameterIncrease = 1e-9;

        DistanceService distanceService(mathService, geometryService, neighborProvider);
        distanceService.SetContext(*context);
        distanceService.SetParticles(*particles);

        Packing& particlesRef = *particles;
        do
        {
            maxDiameterIncrease = 0;

            for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
            {
                DomainParticle& particle = particlesRef[particleIndex];
                FLOAT_TYPE distanceBetweenSurfaces = distanceService.GetDistanceToNearestSurface(particleIndex) - 0.5 * particle.diameter;
                FLOAT_TYPE diameterIncrease = 0.5 * distanceBetweenSurfaces;
                particle.diameter += diameterIncrease;

                if (diameterIncrease > maxDiameterIncrease)
                {
                    maxDiameterIncrease = diameterIncrease;
                }
            }

            printf("maxDiameterIncrease is %g\n", maxDiameterIncrease);
        }
        while (maxDiameterIncrease > minAllowedDiameterIncrease);
    }
}

#endif

