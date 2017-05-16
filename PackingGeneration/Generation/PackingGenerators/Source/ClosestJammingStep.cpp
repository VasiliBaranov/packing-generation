// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include <stdio.h>
#include <boost/numeric/odeint.hpp>
#include "../Headers/ClosestJammingStep.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/Constants.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/IClosestPairProvider.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/Geometries/Headers/IGeometry.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;
using namespace boost::numeric::odeint;

namespace PackingGenerators
{
    ClosestJammingStep::ClosestJammingStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            PackingServices::IClosestPairProvider* closestPairProvider,
            MathService* mathService) :
            BasePackingStep(geometryService, neighborProvider, closestPairProvider, mathService),
            closestJammingVelocityProvider(mathService)
    {
        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = true;

        maxTimeStep = -1.0;
        integrationTimeStep = 1e-9;
    }

    ClosestJammingStep::~ClosestJammingStep()
    {

    }

    const FLOAT_TYPE ClosestJammingStep::GetBondThreshold() const
    {
        return bondsProvider.GetBondThreshold();
    }

    void ClosestJammingStep::SetBondThreshold(Core::FLOAT_TYPE value)
    {
        bondsProvider.SetBondThreshold(value);
    }

    void ClosestJammingStep::SetParticles(Packing* particles)
    {
        BasePackingStep::SetParticles(particles);
        ParticlePair closestPair = closestPairProvider->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        bondsProvider.Reset(config->particlesCount);

        particleVelocities.clear();
        particleVelocities.resize(config->particlesCount);

        bondsProvider.UpdateBonds(*neighborProvider, *mathService, *particles, innerDiameterRatio, false);
        startBondsCountForIntegrationTimeStep = bondsProvider.GetBonds().size();
    }

    void ClosestJammingStep::DisplaceParticles()
    {
        initialInnerDiameterRatio = innerDiameterRatio;
        clock_t linearSystemSolutionTime = FillVelocities();
        FLOAT_TYPE timeStep = FindBestMovementTime();

        int odeCyclesCount = 0;
        if (timeStep <= integrationTimeStep)
        {
            MoveParticles(timeStep);
            innerDiameterRatio += timeStep;
        }
        else
        {
            odeCyclesCount = DisplaceParticlesForLongTime();
        }

        // Remove intersections at each iteration
        innerDiameterRatio = sqrt(closestPairProvider->FindClosestPair().normalizedDistanceSquare);

        // Search for new contacts, update bonds and bonds per particle
        BondsProvider::Statistics statistics = bondsProvider.UpdateBonds(*neighborProvider, *mathService, *particles, innerDiameterRatio, false);
        if (statistics.gapsCount > 0)
        {
            printf("BROKEN: broken bonds count is %d, mean gap length is %g\n", statistics.gapsCount, statistics.meanGapLength);
        }
        if (statistics.intersectionsCount > 0)
        {
            printf("INTERSECTIONS: intersections count is %d, mean intersection length is %g\n", statistics.intersectionsCount, statistics.meanIntersectionLength);
        }

        printf("Bonds count: %d; bond pairs count: %d; time step: %g; slae solution time: %g (s); ode cycles count: %d; integration time step: %g\n",
                bondsProvider.GetBonds().size(), bondsProvider.GetBondPairsCount(), timeStep, linearSystemSolutionTime / static_cast<FLOAT_TYPE>(CLOCKS_PER_SEC), odeCyclesCount, integrationTimeStep);

        FixIntersections(statistics);
    }

    // TODO: merge somehow with DoBinarySearchForCollision
    int ClosestJammingStep::DisplaceParticlesForLongTime()
    {
        // OdeObserver is passed by value, that's why need to store reference to odeCyclesCount.
        int odeCyclesCount = 0;
        OdeObserver observer(&odeCyclesCount, this);

        ParticleSystemForODE odeSystem(this);
        vector<FLOAT_TYPE> combinedParticleCoordinates(DIMENSIONS * config->particlesCount);
        UpdateCombinedCoordinates(&combinedParticleCoordinates);

        FLOAT_TYPE maxTime = maxTimeStep > 0 ? innerDiameterRatio + maxTimeStep : MAX_FLOAT_VALUE;

        try
        {
            // Solve ODE to reach the nextCollisionTime
//            typedef runge_kutta_dopri5< vector<FLOAT_TYPE> > TStepper;
//            result_of::make_controlled<TStepper>::type stepper = make_controlled(bondsProvider.GetBondThreshold(), bondsProvider.GetBondThreshold(), TStepper());
//            integrate_adaptive(stepper, odeSystem, combinedParticleCoordinates, innerDiameterRatio, maxTime, integrationTimeStep, observer);

//            bulirsch_stoer< vector<FLOAT_TYPE> > stepper(bondsProvider.GetBondThreshold(), bondsProvider.GetBondThreshold());
//            integrate_adaptive(stepper, odeSystem, combinedParticleCoordinates, innerDiameterRatio, maxTime, integrationTimeStep, observer);

            typedef runge_kutta_dopri5< vector<FLOAT_TYPE> > TStepper;
            integrate_const(TStepper(), odeSystem, combinedParticleCoordinates, innerDiameterRatio, maxTime, integrationTimeStep, observer);

            // Integration may end normally only if maxTimeStep is specified and reached.
            // Observer is called even for the very last time, therefore coordinates and velocities are correctly updated.
            // In case observer is not called for the last time.
            if (maxTime != MAX_FLOAT_VALUE && maxTime > innerDiameterRatio)
            {
                observer(combinedParticleCoordinates, maxTime);
            }
        }
        catch (const CollisionMissedException& e)
        {
            printf((e.GetMessage() + "\n").c_str());
            DoBinarySearchForCollision(e.nextCollisionTimeEstimate, &observer);
        }
        catch (const CollisionIsNearException& e)
        {
            printf((e.GetMessage() + "\n").c_str());
            // All the velocities are set in the observer and are correct.
            MoveParticles(e.nextCollisionTime - innerDiameterRatio);
            innerDiameterRatio = e.nextCollisionTime;
        }
        return odeCyclesCount;
    }

    void ClosestJammingStep::DoBinarySearchForCollision(FLOAT_TYPE nextCollisionTimeEstimate, OdeObserver* observer)
    {
        printf("DoBinarySearchForCollision; innerDiameterRatio: %17.15g, nextCollisionTimeEstimate: %17.15g\n", innerDiameterRatio, nextCollisionTimeEstimate);
        if ((nextCollisionTimeEstimate - innerDiameterRatio) < integrationTimeStep)
        {
            printf("(nextCollisionTimeEstimate - innerDiameterRatio) < integrationTimeStep\n");
            MoveParticles(nextCollisionTimeEstimate - innerDiameterRatio);
            innerDiameterRatio = nextCollisionTimeEstimate;
            return;
        }

        // Do binary search for collision with support for missing collision detection in the course of integration as well.
        ParticleSystemForODE odeSystem(this);
        vector<FLOAT_TYPE> combinedParticleCoordinates(DIMENSIONS * config->particlesCount);
        UpdateCombinedCoordinates(&combinedParticleCoordinates);

        // Integrate from last valid time (always innerDiameterRatio) to half between valid and invalid times
        FLOAT_TYPE middleTime = (innerDiameterRatio + nextCollisionTimeEstimate) * 0.5;
        try
        {
            // Solve ODE to reach the middleTime
//            typedef runge_kutta_dopri5< vector<FLOAT_TYPE> > TStepper;
//            result_of::make_controlled<TStepper>::type stepper = make_controlled(bondsProvider.GetBondThreshold(), bondsProvider.GetBondThreshold(), TStepper());
//            integrate_adaptive(stepper, odeSystem, combinedParticleCoordinates, innerDiameterRatio, middleTime, integrationTimeStep, observer);

//            bulirsch_stoer< vector<FLOAT_TYPE> > stepper(bondsProvider.GetBondThreshold(), bondsProvider.GetBondThreshold());
//            integrate_adaptive(stepper, odeSystem, combinedParticleCoordinates, innerDiameterRatio, middleTime, integrationTimeStep, observer);

            typedef runge_kutta_dopri5< vector<FLOAT_TYPE> > TStepper;
            integrate_const(TStepper(), odeSystem, combinedParticleCoordinates, innerDiameterRatio, middleTime, integrationTimeStep, *observer);

            // Observer was not called for the last time
            if (middleTime > innerDiameterRatio)
            {
                (*observer)(combinedParticleCoordinates, middleTime);
            }

            printf("innerDiameterRatio after successful integration: %17.15g\n", innerDiameterRatio);
            // Integrated successfully. Observer was also called for the last step,
            // so innerDiameterRatio, particle coordinates, bonds, and particle velocities have correct values (innerDiameterRatio = middleTime),
            // collision has not happened in the middle time.
            // Last valid time is the current time, nextCollisionTimeEstimate is the same.
            DoBinarySearchForCollision(nextCollisionTimeEstimate, observer);
        }

        // If integrated not successfully:
        // a. if collision missed, it can not be too close to the last valid time (otherwise the near exception would be fired).
        catch (const CollisionMissedException& e)
        {
            // innerDiameterRatio, particle coordinates, bonds, and particle velocities have correct values (innerDiameterRatio = last time before collision)
            printf((e.GetMessage() + "\n").c_str());
            DoBinarySearchForCollision(e.nextCollisionTimeEstimate, observer);
        }
        // b. if collision is near, move particles
        catch (const CollisionIsNearException& e)
        {
            printf((e.GetMessage() + "\n").c_str());
            // All the velocities are set in the observer and are correct.
            MoveParticles(e.nextCollisionTime - innerDiameterRatio);
            innerDiameterRatio = e.nextCollisionTime;
        }
    }

    void ClosestJammingStep::FixIntersections(BondsProvider::Statistics statistics)
    {
        ParticlePair closestPair = closestPairProvider->FindClosestPair();
        const FLOAT_TYPE tolerance = 1.0 - 5.0 * bondsProvider.GetBondThreshold();
        bool intersectionsExist = closestPair.normalizedDistanceSquare < innerDiameterRatio * innerDiameterRatio * tolerance * tolerance;
        if (intersectionsExist)
        {
            printf("WARNING: min normalized distance %f is lower than inner diameter ratio %f. Particles pair: %d and %d.\n",
                    std::sqrt(closestPair.normalizedDistanceSquare),
                    innerDiameterRatio,
                    closestPair.firstParticleIndex,
                    closestPair.secondParticleIndex);
        }

        bool errorIsLarge =
                statistics.meanGapLength > bondsProvider.GetBondThreshold() * 5.0 ||
                statistics.meanIntersectionLength > bondsProvider.GetBondThreshold() * 5.0 ||
                intersectionsExist;
        if (!errorIsLarge)
        {
            return;
        }

        printf("WARNING: meanGapLength or meanIntersectionLength are too large. Updating innerDiameterRatio and bonds.\n");
        // The last bond is added with errors. The last but one is not, because otherwise bonds would be recalculated.
        int endBondsCountForIntegrationTimeStep = bondsProvider.GetBonds().size() - 1;
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
        bondsProvider.UpdateBonds(*neighborProvider, *mathService, *particles, innerDiameterRatio, true);

        UpdateIntegrationTimeStep(endBondsCountForIntegrationTimeStep);
    }

    void ClosestJammingStep::UpdateIntegrationTimeStep(int endBondsCountForIntegrationTimeStep)
    {
        FLOAT_TYPE addedBondsCount = endBondsCountForIntegrationTimeStep - startBondsCountForIntegrationTimeStep;
        bool errorGrowsTooQuickly = addedBondsCount < 10;

        startBondsCountForIntegrationTimeStep = endBondsCountForIntegrationTimeStep;

        // After division by 2 integrationTimeStep should be at least 1e-14, as final particle diameter is about 1
        const FLOAT_TYPE minIntegrationTimeStep = 2e-14;
        bool shouldUpdateIntegrationTimeStep = errorGrowsTooQuickly && integrationTimeStep > minIntegrationTimeStep;
        if (shouldUpdateIntegrationTimeStep)
        {
            integrationTimeStep *= 0.5;
            printf("WARNING: error grows too quickly. Updated integrationTimeStep to %g.\n", integrationTimeStep);
        }

        if (addedBondsCount <= 0 && integrationTimeStep <= minIntegrationTimeStep)
        {
            throw InvalidOperationException("Particles do not grow during integration, integrationTimeStep is too low to be decreased further.");
        }
    }

    clock_t ClosestJammingStep::FillVelocities()
    {
        clock_t solutionTime = closestJammingVelocityProvider.FillVelocities(*neighborProvider, bondsProvider, *particles, innerDiameterRatio, &particleVelocities);
        return solutionTime;
    }

    void ClosestJammingStep::UpdateCombinedCoordinates(vector<FLOAT_TYPE>* combinedParticleCoordinates)
    {
        vector<FLOAT_TYPE>& combinedParticleCoordinatesRef = *combinedParticleCoordinates;
        const Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];
            for (int i = 0; i < DIMENSIONS; ++i)
            {
                combinedParticleCoordinatesRef[particleIndex * DIMENSIONS + i] = particle.coordinates[i];
            }
        }
    }

    void ClosestJammingStep::UpdateCombinedVelocities(vector<FLOAT_TYPE>* combinedVelocities)
    {
        vector<FLOAT_TYPE>& combinedVelocitiesRef = *combinedVelocities;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const Core::SpatialVector& velocity = particleVelocities[particleIndex];
            for (int i = 0; i < DIMENSIONS; ++i)
            {
                combinedVelocitiesRef[particleIndex * DIMENSIONS + i] = velocity[i];
            }
        }
    }

    void ClosestJammingStep::UpdateParticleCoordinates(const vector<FLOAT_TYPE>& combinedParticleCoordinates)
    {
        Model::Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DomainParticle& particle = particlesRef[particleIndex];

            closestPairProvider->StartMove(particleIndex);
            neighborProvider->StartMove(particleIndex);

            for (int i = 0; i < DIMENSIONS; ++i)
            {
                particle.coordinates[i] = combinedParticleCoordinates[particleIndex * DIMENSIONS + i];
            }
            geometry->EnsureBoundaries(particle, &particle, innerDiameterRatio);

            neighborProvider->EndMove();
            closestPairProvider->EndMove();
        }
    }

    void ClosestJammingStep::ProvideODEDerivative(const vector<FLOAT_TYPE>& x, vector<FLOAT_TYPE>* dxdt, const FLOAT_TYPE t)
    {
        // copy x into particles, update innerDiameterRatio
        UpdateParticleCoordinates(x);
//        FLOAT_TYPE previousTime = innerDiameterRatio;
        innerDiameterRatio = t;

        FillVelocities();
        UpdateCombinedVelocities(dxdt);

//        ////////////////
//        // Search for new contacts, update bonds and bonds per particle
//        Nullable<FLOAT_TYPE> meanGapLength = UpdateContacts();
//
//        ParticlePair closestPair = closestPairProvider->FindClosestPair();
//        const FLOAT_TYPE tolerance = 1.0 - bondThreshold;
//        if (closestPair.normalizedDistanceSquare < innerDiameterRatio * innerDiameterRatio * tolerance * tolerance)
//        {
//            printf("ODE: WARNING: min normalized distance %f is lower than inner diameter ratio %f. Particles pair: %d and %d; bond existed: %d\n",
//                    std::sqrt(closestPair.normalizedDistanceSquare),
//                    innerDiameterRatio,
//                    closestPair.firstParticleIndex,
//                    closestPair.secondParticleIndex,
//                    ParticlesShareBond(closestPair.firstParticleIndex, closestPair.secondParticleIndex));
//        }
//
//        printf("ODE: Bonds count: %d; bond pairs count: %d; time step: %g; slae solution time: %g (s); ode delta t: %g\n",
//                bonds.size(), bondPairs.size(), timeStep, linearSystemSolutionTime / static_cast<FLOAT_TYPE>(CLOCKS_PER_SEC), t - previousTime);
//        ////////////////
    }

    FLOAT_TYPE ClosestJammingStep::FindBestMovementTime()
    {
        FLOAT_TYPE movementTime = closestJammingVelocityProvider.FindBestMovementTime();
        if (maxTimeStep > 0 && (movementTime + innerDiameterRatio - initialInnerDiameterRatio > maxTimeStep))
        {
            movementTime = initialInnerDiameterRatio + maxTimeStep - innerDiameterRatio;
        }
        return movementTime;
    }

    void ClosestJammingStep::MoveParticles(FLOAT_TYPE timeStep)
    {
        Model::Packing& particlesRef = *particles;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            DomainParticle& particle = particlesRef[particleIndex];
            if (bondsProvider.GetBondIndexesPerParticle()[particleIndex].size() == 0)
            {
                continue;
            }

            closestPairProvider->StartMove(particleIndex);
            neighborProvider->StartMove(particleIndex);

            SpatialVector displacement;
            VectorUtilities::MultiplyByValue(particleVelocities[particleIndex], timeStep, &displacement);
            VectorUtilities::Add(particle.coordinates, displacement, &particle.coordinates);
            geometry->EnsureBoundaries(particle, &particle, innerDiameterRatio);

            neighborProvider->EndMove();
            closestPairProvider->EndMove();
        }
    }

    void ClosestJammingStep::ResetGeneration()
    {
        throw InvalidOperationException("ResetGeneration does nothing for ClosestJammingStep. Always set maxRunsCount = 1 for LS.");
    }

    bool ClosestJammingStep::ShouldContinue() const
    {
         // 3 degrees of freedom are diminished as one particle can always be considered fixed.
         return bondsProvider.GetBonds().size() < static_cast<size_t>(DIMENSIONS * (config->particlesCount - 1));

//        // Expected bonds count n = DIMENSIONS * (particlesCount - 1) = 30 000 - 3 = 29997
//        // Expected coordination number c = 2 * n / particlesCount = 2 * n - 1 / particlesCount = 5.9994 ~ 6 = 2 * n / particlesCount
//        // Acceptable coordination number = 5.98
//        // Acceptable bonds count n = 5.98 / 2 * particlesCount = 29900. This difference will matter a lot (each iteration may consume > 1 minute)
//        size_t acceptableBondsCount = DIMENSIONS * (5.98 / 6.0) * config->particlesCount;
//        return bondsProvider.GetBonds().size() < acceptableBondsCount;
    }
}

