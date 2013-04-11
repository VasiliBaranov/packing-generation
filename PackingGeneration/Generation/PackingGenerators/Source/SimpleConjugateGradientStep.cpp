// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/SimpleConjugateGradientStep.h"

#ifdef GSL_AVAILABLE

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
    SimpleConjugateGradientStep::SimpleConjugateGradientStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            MathService* mathService,
            IPairPotential* pairPotential,
            IEnergyService* energyService) :
            BaseConjugateGradientStep(geometryService, neighborProvider, mathService, pairPotential, energyService)
    {
        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = true;

//         outerDiameterRatio = 1.0 + 1.0e-3;
        outerDiameterRatio = 1.0;
    }

    void SimpleConjugateGradientStep::SetParticles(Packing* particles)
    {
        BaseConjugateGradientStep::SetParticles(particles);
        shouldContinue = true;

        InitializeMinimization();
    }

    bool SimpleConjugateGradientStep::ShouldContinue() const
    {
        return shouldContinue;
    }

    void SimpleConjugateGradientStep::DisplaceParticles()
    {
        int errorStatus = gsl_multimin_fdfminimizer_iterate(minimizer);
        shouldContinue = (errorStatus == GSL_SUCCESS);

        // iteration is not making progress towards solution
        if (errorStatus == GSL_ENOPROG)
        {
            printf("Iteration is not making progress towards solution\n");
        }

        if (shouldContinue)
        {
            // 1e-3 is global tolerance.
            const FLOAT_TYPE globalTolerance = 1e-15;
            int result = gsl_multimin_test_gradient(minimizer->gradient, globalTolerance);
            shouldContinue = (result == GSL_CONTINUE);
        }

        // When minimizing energy we update the inner diameter ratio, but that may not be perfectly correct,
        // as we don't know the details of algorithm, so at the end we calculate it once again.
        if (!shouldContinue)
        {
            //ParticlePair closestPair = distanceService->FindClosestPair();
            //innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
            printf("Final potential energy: %g\n", minimizer->f);
        }
    }

    void SimpleConjugateGradientStep::ResetOuterDiameterRatio()
    {
        FLOAT_TYPE expectedDensity = 0.6522;
        outerDiameterRatio = pow(expectedDensity / (1.0 - theoreticalPorosity), 1.0 / 3.0);
    }
}

#endif

