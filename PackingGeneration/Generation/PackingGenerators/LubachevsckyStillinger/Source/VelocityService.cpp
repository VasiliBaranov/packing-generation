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
        int particlesCount = particles->size();
        vector<MovingParticle>& particlesRef = *particles;

        // Rescale factor is chosen to reestablish temperature of the system
        FLOAT_TYPE expectedKineticEnergy = GetExpectedKineticEnergy(particlesCount);
        FLOAT_TYPE rescaleFactor = sqrt(expectedKineticEnergy / actualKineticEnergy);
        for (ParticleIndex i = 0; i < particlesCount; ++i)
        {
            MovingParticle* particle = &particlesRef[i];

            // As far as we update velocities, we should synchronize particles with the current time.
            SynchronizeParticleWithCurrentTime(currentTime, particle);
            VectorUtilities::MultiplyByValue(particle->velocity, rescaleFactor, &particle->velocity);
        }
    }

    void VelocityService::SynchronizeParticleWithCurrentTime(FLOAT_TYPE currentTime, MovingParticle* particle) const
    {
        SpatialVector displacement;

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

    void VelocityService::FillVelocities(vector<MovingParticle>* particles) const
    {
        int particlesCount = particles->size();
        vector<MovingParticle>& particlesRef = *particles;
        SpatialVector totalImpetus;
        VectorUtilities::InitializeWith(&totalImpetus, 0.0);

        for (ParticleIndex particleIndex = 0; particleIndex < particlesCount; ++particleIndex)
        {
             MovingParticle& movingParticle = particlesRef[particleIndex];
             FillInitialVelocity(&movingParticle.velocity);

             SpatialVector impetus;
             VectorUtilities::MultiplyByValue(movingParticle.velocity, mass, &impetus);
             VectorUtilities::Add(totalImpetus, impetus, &totalImpetus);
        }

        SpatialVector centerOfMassVelocity;
        VectorUtilities::DivideByValue(totalImpetus, mass * particlesCount, &centerOfMassVelocity);
        // printf("Velocity of center of mass: %f %f %f\n", centerMassVelocity[Axis::X], centerMassVelocity[Axis::Y], centerMassVelocity[Axis::Z]);

        FLOAT_TYPE centerOfMassVelocityLength = VectorUtilities::GetLength(centerOfMassVelocity);
        // TODO: calculate max acceptable center mass velocity better (confidence intervals by std; std depends on temperature)
        if (centerOfMassVelocityLength > 500.0 / mass / particlesCount)
        {
            printf("WARNING: Velocity of center of mass not zero.\n");
        }

        // Change the reference system to the center of mass; make velocities have zero mean. The shape of the distribution (gaussian) is preserved.
        for (ParticleIndex particleIndex = 0; particleIndex < particlesCount; ++particleIndex)
        {
             MovingParticle& movingParticle = particlesRef[particleIndex];
             VectorUtilities::Subtract(movingParticle.velocity, centerOfMassVelocity, &movingParticle.velocity);
        }

        // Ensure exact temperature (it may not be exact due to random generator noise; and due to the change of the system of reference).
        // The shape of the distribution (gaussian) is preserved.
        FLOAT_TYPE actualKineticEnergy = GetActualKineticEnergy(*particles);
        FLOAT_TYPE currentTime = 0.0;
        RescaleVelocities(currentTime, actualKineticEnergy, particles);
    }

    // E = N i k T / 2, where N - number of particles, i - degrees of freedom
    FLOAT_TYPE VelocityService::GetActualTemperature(FLOAT_TYPE kineticEnergy, int particlesCount) const
    {
        const FLOAT_TYPE degreeOfFreedom = 3;
        return 2.0 * kineticEnergy / particlesCount / degreeOfFreedom / boltzmannConstant;
    }

    FLOAT_TYPE VelocityService::GetExpectedKineticEnergy(int particlesCount) const
    {
        const FLOAT_TYPE degreeOfFreedom = 3;
        return particlesCount * degreeOfFreedom * boltzmannConstant * temperature / 2.0;
    }
}

