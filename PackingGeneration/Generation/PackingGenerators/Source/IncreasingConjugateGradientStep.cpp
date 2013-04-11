// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/IncreasingConjugateGradientStep.h"
#include "Generation/Model/Headers/Config.h"

#ifdef GSL_AVAILABLE

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
    const FLOAT_TYPE IncreasingConjugateGradientStep::relativeEnergyTolerance = 1e-16;
    const FLOAT_TYPE IncreasingConjugateGradientStep::minEnergyPerParticle= 1e-16;

    IncreasingConjugateGradientStep::IncreasingConjugateGradientStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IPairPotential* pairPotential,
            IEnergyService* energyService) :
            BaseConjugateGradientStep(geometryService, neighborProvider, mathService, pairPotential, energyService)
    {
        isOuterDiameterChanging = true;
        canOvercomeTheoreticalDensity = true;
    }

    void IncreasingConjugateGradientStep::SetParticles(Packing* particles)
    {
        BaseConjugateGradientStep::SetParticles(particles);
        shouldContinue = true;
        currentContractionRate = generationConfig->contractionRate;
    }

    void IncreasingConjugateGradientStep::DisplaceParticles()
    {
        // So that initial potential energy is valid after any outer diameter update or reset.
        InitializeMinimization();
        FLOAT_TYPE previousEnergy = minimizer->f;

        while (true)
        {
            int errorStatus = gsl_multimin_fdfminimizer_iterate(minimizer);
            FLOAT_TYPE currentEnergy = minimizer->f;
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
                const FLOAT_TYPE globalTolerance = 1e-5;
                int result = gsl_multimin_test_gradient(minimizer->gradient, globalTolerance * potentialNormalizer);
                localShouldContinue = (result == GSL_CONTINUE);
            }

            if (!localShouldContinue)
            {
                printf("Stop iteration. Gradient too small. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, GetEnergyPerParticle());
                break;
            }

//            if (localShouldContinue)
//            {
//                bool energyStationary = (std::abs(currentEnergy - previousEnergy) / currentEnergy) < relativeEnergyTolerance;
//                bool energyZero = GetEnergyPerParticle() < minEnergyPerParticle;
//                localShouldContinue = energyStationary || energyZero;
//            }

//            if (!localShouldContinue)
//            {
//                printf("Stop iteration. Energy is stationary or zero. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, GetEnergyPerParticle());
//                break;
//            }

            previousEnergy = currentEnergy;
        }

        FLOAT_TYPE energyPerParticle = GetEnergyPerParticle();
        shouldContinue = energyPerParticle < minEnergyPerParticle || energyPerParticle > 2 * minEnergyPerParticle;

        if (!shouldContinue)
        {
            // In this algorithm we allow particles to intersect a little bit, so we make the final inner diameters equal to the outer ones
            innerDiameterRatio = outerDiameterRatio;
            printf("Stop algorithm. Inner diameter ratio: %g; Energy per particle: %g\n", innerDiameterRatio, energyPerParticle);
        }
        else
        {
            UpdateOuterDiameterRatio();
        }
    }

    FLOAT_TYPE IncreasingConjugateGradientStep::GetEnergyPerParticle() const
    {
        FLOAT_TYPE energyPerParticle = minimizer->f / potentialNormalizer / config->particlesCount;
        return energyPerParticle;
    }

    void IncreasingConjugateGradientStep::UpdateOuterDiameterRatio()
    {
        FLOAT_TYPE energyPerParticle = GetEnergyPerParticle();
        bool outerDiameterDirectionIsUp;
        if (energyPerParticle < minEnergyPerParticle)
        {
            outerDiameterDirectionIsUp = true;
        }
        if (energyPerParticle > 2 * minEnergyPerParticle)
        {
            outerDiameterDirectionIsUp = false;
        }

        if (previousOuterDiameterDirectionIsUp != outerDiameterDirectionIsUp)
        {
            currentContractionRate *= 0.5;
        }

        FLOAT_TYPE multiplier = outerDiameterDirectionIsUp ? 1 : -1;
        outerDiameterRatio += multiplier * currentContractionRate;
        previousOuterDiameterDirectionIsUp = outerDiameterDirectionIsUp;
    }

    void IncreasingConjugateGradientStep::ResetOuterDiameterRatio()
    {
        previousOuterDiameterDirectionIsUp = true;
        outerDiameterRatio = innerDiameterRatio;
    }

    bool IncreasingConjugateGradientStep::ShouldContinue() const
    {
        return shouldContinue;
    }
}

#endif

