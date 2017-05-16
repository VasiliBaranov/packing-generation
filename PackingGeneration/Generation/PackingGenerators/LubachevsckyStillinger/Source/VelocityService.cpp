// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VelocityService.h"
#include <stdio.h>
#include "Core/Headers/Math.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    const FLOAT_TYPE VelocityService::mass = 1.0;
    const FLOAT_TYPE VelocityService::boltzmannConstant = 1.0;

    VelocityService::VelocityService()
    {
        temperature = 0.2;
    }

    void VelocityService::RescaleVelocities(FLOAT_TYPE currentTime, FLOAT_TYPE actualKineticEnergy, vector<MovingParticle>* particles) const
    {
        // Rescale factor is chosen to reestablish the temperature of the system
        FLOAT_TYPE expectedKineticEnergy = GetExpectedKineticEnergy(*particles);
        FLOAT_TYPE rescaleFactor = sqrt(expectedKineticEnergy / actualKineticEnergy);

        int particlesCount = particles->size();
        vector<MovingParticle>& particlesRef = *particles;
        for (ParticleIndex i = 0; i < particlesCount; ++i)
        {
            MovingParticle* particle = &particlesRef[i];

            // As far as we update velocities, we should synchronize particles with the current time.
            // Don't check for immobility, as zero velocities will be rescaled to zero velocities.
            SynchronizeParticleWithCurrentTime(currentTime, particle);
            VectorUtilities::MultiplyByValue(particle->velocity, rescaleFactor, &particle->velocity);
        }
    }

    void VelocityService::SynchronizeParticleWithCurrentTime(FLOAT_TYPE currentTime, MovingParticle* particle) const
    {
        SpatialVector displacement;

        // Don't check for immobility, as immobile particles have zero velocities and won't be moved.
        VectorUtilities::MultiplyByValue(particle->velocity, currentTime - particle->lastEventTime, &displacement);
        VectorUtilities::Add(particle->coordinates, displacement, &particle->coordinates);

        // May not ensure periodic boundaries as they will be ensured in wall transfer events
        particle->lastEventTime = currentTime;
    }

    FLOAT_TYPE VelocityService::GetActualKineticEnergy(const vector<MovingParticle>& particles) const
    {
        int particlesCount = particles.size();
        FLOAT_TYPE kineticEnergy = 0;
        for (ParticleIndex i = 0; i < particlesCount; ++i)
        {
            // Don't check for immobility, as immobile particles have zero velocities and won't influence kinetic energy.
            // E += 0.5*s[i].m*s[i].v.norm_squared();
            const MovingParticle& particle = particles[i];
            FLOAT_TYPE velocitySquare = VectorUtilities::GetSelfDotProduct(particle.velocity);
            kineticEnergy += 0.5 * mass * velocitySquare;
        }

        return kineticEnergy;
    }

    // See http://en.wikipedia.org/wiki/Maxwell%E2%80%93Boltzmann_distribution
    void VelocityService::FillInitialVelocity(SpatialVector* velocity) const
    {
        SpatialVector& velocityRef = *velocity;
        const FLOAT_TYPE mean = 0.0;
        const FLOAT_TYPE standardDeviation = sqrt(boltzmannConstant * temperature / mass);

        for (int i = 0; i < DIMENSIONS; ++i)
        {
            velocityRef[i] = Math::GetNextGaussianRandom(mean, standardDeviation);
            // printf("%f\n", velocity[i]);
        }
    }

    int VelocityService::GetMobileParticlesCount(const std::vector<MovingParticle>& particles) const
    {
        int particlesCount = particles.size();
        int mobileParticlesCount = 0;
        for (ParticleIndex particleIndex = 0; particleIndex < particlesCount; ++particleIndex)
        {
            const MovingParticle& movingParticle = particles[particleIndex];
            if (!movingParticle.isImmobile)
            {
                mobileParticlesCount++;
            }
        }

        return mobileParticlesCount;
    }

    void VelocityService::FillVelocities(vector<MovingParticle>* particles) const
    {
        int mobileParticlesCount = GetMobileParticlesCount(*particles);
        int particlesCount = particles->size();

        vector<MovingParticle>& particlesRef = *particles;

        SpatialVector totalImpetus;
        VectorUtilities::InitializeWith(&totalImpetus, 0.0);

        for (ParticleIndex particleIndex = 0; particleIndex < particlesCount; ++particleIndex)
        {
             MovingParticle& movingParticle = particlesRef[particleIndex];

             if (movingParticle.isImmobile)
             {
                 VectorUtilities::InitializeWith(&movingParticle.velocity, 0.0);
             }
             else
             {
                 FillInitialVelocity(&movingParticle.velocity);

                 SpatialVector impetus;
                 VectorUtilities::MultiplyByValue(movingParticle.velocity, mass, &impetus);
                 VectorUtilities::Add(totalImpetus, impetus, &totalImpetus);
             }
        }

        SpatialVector centerOfMassVelocity;
        VectorUtilities::DivideByValue(totalImpetus, mass * mobileParticlesCount, &centerOfMassVelocity);
        // printf("Velocity of center of mass: %f %f %f\n", centerMassVelocity[Axis::X], centerMassVelocity[Axis::Y], centerMassVelocity[Axis::Z]);

        FLOAT_TYPE centerOfMassVelocityLength = VectorUtilities::GetLength(centerOfMassVelocity);
        // TODO: calculate max acceptable center mass velocity better (confidence intervals by std; std depends on temperature)
        if (centerOfMassVelocityLength > 500.0 / mass / mobileParticlesCount)
        {
            printf("WARNING: Velocity of center of mass not zero.\n");
        }

        // Change the reference system to the center of mass; make velocities have zero mean. The shape of the distribution (gaussian) is preserved.
        for (ParticleIndex particleIndex = 0; particleIndex < particlesCount; ++particleIndex)
        {
             MovingParticle& movingParticle = particlesRef[particleIndex];
             if (!movingParticle.isImmobile)
             {
                 VectorUtilities::Subtract(movingParticle.velocity, centerOfMassVelocity, &movingParticle.velocity);
             }
        }

        // Ensure exact temperature (it may not be exact due to random generator noise; and due to the change of the system of reference).
        // The shape of the distribution (gaussian) is preserved.
        FLOAT_TYPE actualKineticEnergy = GetActualKineticEnergy(*particles);
        FLOAT_TYPE currentTime = 0.0;
        RescaleVelocities(currentTime, actualKineticEnergy, particles);
    }

    // E = N i k T / 2, where N - number of particles, i - degrees of freedom
    FLOAT_TYPE VelocityService::GetActualTemperature(FLOAT_TYPE kineticEnergy, const std::vector<MovingParticle>& particles) const
    {
        int mobileParticlesCount = GetMobileParticlesCount(particles);
        const FLOAT_TYPE degreeOfFreedom = DIMENSIONS;
        return 2.0 * kineticEnergy / mobileParticlesCount / degreeOfFreedom / boltzmannConstant;
    }

    FLOAT_TYPE VelocityService::GetExpectedKineticEnergy(const std::vector<MovingParticle>& particles) const
    {
        int mobileParticlesCount = GetMobileParticlesCount(particles);
        const FLOAT_TYPE degreeOfFreedom = DIMENSIONS;
        return mobileParticlesCount * degreeOfFreedom * boltzmannConstant * temperature / 2.0;
    }
}

