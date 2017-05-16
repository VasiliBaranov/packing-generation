// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_ClosestJammingStep_h
#define Generation_PackingGenerators_Headers_ClosestJammingStep_h

#include "BasePackingStep.h"
#include "Generation/PackingServices/Headers/ClosestJammingVelocityProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/BondsProvider.h"
#include "Core/Headers/Exceptions.h"

namespace PackingGenerators
{
    // Implements an algorithm for searching for the closest jamming density.
    // It's a variation of the initial stage of the Zinchenko algorithm (see section 2.4 in Zinchenko (1994) Algorithm for Random Close Packing of Spheres with Periodic Boundary Conditions.
    // Here we also require the minimization of particles displacement during movement, searching for the velocities is turned into a task of minimization with restrictions (Lagrangian multipliers),
    // which is in turned transformed into a solution of a linear system of equations.
    // It is not yet published anywhere.
    class ClosestJammingStep : public BasePackingStep
    {
    private:
        // Services. TODO: Pass by pointer, when finally needed for testability.
        PackingServices::BondsProvider bondsProvider;
        PackingServices::ClosestJammingVelocityProvider closestJammingVelocityProvider;

        // Working variables
        int startBondsCountForIntegrationTimeStep;
        Core::FLOAT_TYPE initialInnerDiameterRatio;
        std::vector<Core::SpatialVector> particleVelocities;

        class OdeObserver;

    public:
        Core::FLOAT_TYPE maxTimeStep; // used in tests only
        Core::FLOAT_TYPE integrationTimeStep;

    public:
        ClosestJammingStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::IClosestPairProvider* closestPairProvider,
                PackingServices::MathService* mathService);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        OVERRIDE bool ShouldContinue() const;

        const Core::FLOAT_TYPE GetBondThreshold() const;

        void SetBondThreshold(Core::FLOAT_TYPE value);

        ~ClosestJammingStep();

    private:
        int DisplaceParticlesForLongTime();

        void DoBinarySearchForCollision(Core::FLOAT_TYPE nextCollisionTimeEstimate, OdeObserver* observer);

        void FixIntersections(PackingServices::BondsProvider::Statistics statistics);

        void UpdateIntegrationTimeStep(int endBondsCountForIntegrationTimeStep);

        void ProvideODEDerivative(const std::vector<Core::FLOAT_TYPE>& x, std::vector<Core::FLOAT_TYPE>* dxdt, const Core::FLOAT_TYPE t);

        void UpdateCombinedCoordinates(std::vector<Core::FLOAT_TYPE>* combinedParticleCoordinates);

        void UpdateCombinedVelocities(std::vector<Core::FLOAT_TYPE>* combinedVelocities);

        void UpdateParticleCoordinates(const std::vector<Core::FLOAT_TYPE>& combinedParticleCoordinates);

        void MoveParticles(Core::FLOAT_TYPE timeStep);

        clock_t FillVelocities();

        Core::FLOAT_TYPE FindBestMovementTime();

        DISALLOW_COPY_AND_ASSIGN(ClosestJammingStep);

    private:
        class ParticleSystemForODE
        {
        private:
            ClosestJammingStep* closestJammingStep;

        public:
            explicit ParticleSystemForODE(ClosestJammingStep* closestJammingStep)
            {
                this->closestJammingStep = closestJammingStep;
            }

            void operator() (const std::vector<Core::FLOAT_TYPE>& x, std::vector<Core::FLOAT_TYPE>& dxdt, const Core::FLOAT_TYPE t)
            {
                closestJammingStep->ProvideODEDerivative(x, &dxdt, t);
            }
        };

        class CollisionMissedException : public Core::Exception
        {
        public:
            Core::FLOAT_TYPE nextCollisionTimeEstimate;
        public:
            CollisionMissedException(std::string message, Core::FLOAT_TYPE nextCollisionTimeEstimate) : Core::Exception(message)
            {
                this->nextCollisionTimeEstimate = nextCollisionTimeEstimate;
            }
        };

        class CollisionIsNearException : public Core::Exception
        {
        public:
            Core::FLOAT_TYPE nextCollisionTime;

        public:
            CollisionIsNearException(std::string message, Core::FLOAT_TYPE nextCollisionTime) : Core::Exception(message)
            {
                this->nextCollisionTime = nextCollisionTime;
            }
        };

        class OdeObserver
        {
        private:
            ClosestJammingStep* closestJammingStep;
            Core::FLOAT_TYPE previousStep;
            std::vector<Core::FLOAT_TYPE> lastValidCombinedParticleCoordinates;
            std::vector<Core::SpatialVector> lastValidParticleVelocities;
            Core::FLOAT_TYPE lastValidTime;

        public:
            int& callsCount;

            OdeObserver(int* callsCountPtr, ClosestJammingStep* closestJammingStep) : callsCount(*callsCountPtr)
            {
                this->closestJammingStep = closestJammingStep;
                previousStep = 0;
            }

            // If observer throws exception, innerDiameterRatio, particle coordinates, bonds, and particle velocities should have correct and consistent values (i.e. before new bonds occurred).
            void operator()(const std::vector<Core::FLOAT_TYPE>& x, Core::FLOAT_TYPE t)
            {
                printf("Time: %17.15g; time step: %g\n", t, t - previousStep);
                previousStep = t;
                callsCount++;

                closestJammingStep->innerDiameterRatio = t;
                closestJammingStep->UpdateParticleCoordinates(x);

                PackingServices::BondsProvider::Statistics statistics =
                        closestJammingStep->bondsProvider.UpdateBonds(*closestJammingStep->neighborProvider, *closestJammingStep->mathService,
                        *closestJammingStep->particles, closestJammingStep->innerDiameterRatio, false);

                if (statistics.addedBonds.size() > 0)
                {
                    // Roll back to last valid values
                    closestJammingStep->UpdateParticleCoordinates(lastValidCombinedParticleCoordinates);
                    closestJammingStep->particleVelocities = lastValidParticleVelocities;
                    closestJammingStep->innerDiameterRatio = lastValidTime;

                    closestJammingStep->bondsProvider.RemoveBonds(statistics.addedBonds);

                    throw CollisionMissedException("Bonds added", t);
                }

                // No bonds added. Check if the collision is near enough to integrate by simple Euler explicit scheme.
                closestJammingStep->FillVelocities();
                Core::FLOAT_TYPE timeStep = closestJammingStep->FindBestMovementTime();
                if (timeStep < closestJammingStep->integrationTimeStep)
                {
                    // innerDiameterRatio, particle coordinates and particle velocities have correct values
                    throw CollisionIsNearException("Collision is near, can simply move particles by this time step", closestJammingStep->innerDiameterRatio + timeStep);
                }

                // Integration will continue. Save this vector as the last valid coordinates vector to roll back to in case CollisionMissedException is fired above.
                lastValidCombinedParticleCoordinates = x;
                lastValidParticleVelocities = closestJammingStep->particleVelocities;
                lastValidTime = closestJammingStep->innerDiameterRatio;

                ////////////////
                if (statistics.gapsCount > 0)
                {
                    printf("ODE BROKEN: broken bonds count is %d, mean gap length is %g\n", statistics.gapsCount, statistics.meanGapLength);
                }
                if (statistics.intersectionsCount > 0)
                {
                    printf("ODE INTERSECTIONS: intersections count is %d, mean intersection length is %g\n", statistics.intersectionsCount, statistics.meanIntersectionLength);
                }
                ////////////////
            }
        };
    };
}

#endif /* Generation_PackingGenerators_Headers_ClosestJammingStep_h */
