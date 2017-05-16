// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/DecreasingConjugateGradientStep.h"
#include "Generation/Model/Headers/Config.h"

#ifdef GSL_AVAILABLE

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
    const FLOAT_TYPE DecreasingConjugateGradientStep::NOMINAL_DENSITY_RATIO = 1.2;

    DecreasingConjugateGradientStep::DecreasingConjugateGradientStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IPairPotential* pairPotential,
            IEnergyService* energyService) :
            BaseConjugateGradientStep(geometryService, neighborProvider, mathService, pairPotential, energyService)
    {
        isOuterDiameterChanging = true;
        canOvercomeTheoreticalDensity = true;
    }

    void DecreasingConjugateGradientStep::DisplaceParticles()
    {
        // So that initial potential energy is valid after any outer diameter update or rest.
        InitializeMinimization();

        while (true)
        {
            int errorStatus = gsl_multimin_fdfminimizer_iterate(minimizer);
            bool shouldContinue = (errorStatus == GSL_SUCCESS);

            // Iteration is not making progress towards solution
            if (errorStatus == GSL_ENOPROG)
            {
                printf("Iteration is not making progress towards solution\n");
                break;
            }

            if (shouldContinue)
            {
                const FLOAT_TYPE globalTolerance = 1e-5;
                int result = gsl_multimin_test_gradient(minimizer->gradient, globalTolerance * potentialNormalizer);
                shouldContinue = (result == GSL_CONTINUE);
            }

            if (!shouldContinue)
            {
                printf("Inner diameter ratio: %g; Final potential energy: %g\n", innerDiameterRatio, minimizer->f);
                break;
            }
        }

        UpdateOuterDiameterRatio();
    }

    void DecreasingConjugateGradientStep::UpdateOuterDiameterRatio()
    {
        FLOAT_TYPE outerPorosity = CalculateCurrentPorosity(outerDiameterRatio);
        FLOAT_TYPE innerPorosity = CalculateCurrentPorosity(innerDiameterRatio);

        // Formula from the Bezrukov, Bargiel, Stoyan (2002)
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

    void DecreasingConjugateGradientStep::ResetOuterDiameterRatio()
    {
        outerDiameterRatio = pow(NOMINAL_DENSITY_RATIO, 1.0 / DIMENSIONS);
        initialOuterDiameterRatio = outerDiameterRatio;
    }
}

#endif

