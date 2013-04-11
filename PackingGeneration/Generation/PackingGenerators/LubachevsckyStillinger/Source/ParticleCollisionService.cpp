// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ParticleCollisionService.h"

#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "../Headers/Types.h"

using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace PackingGenerators
{
    ParticleCollisionService::ParticleCollisionService(MathService* mathService)
    {
        this->mathService = mathService;
    }

    void ParticleCollisionService::Initialize(FLOAT_TYPE initialInnerDiameterRatio, FLOAT_TYPE ratioGrowthRate)
    {
        this->initialInnerDiameterRatio = initialInnerDiameterRatio;
        this->ratioGrowthRate = ratioGrowthRate;
    }

    // See Lubachevscky, Stillinger (1990) Geometric properties of random disk packings
    FLOAT_TYPE ParticleCollisionService::FillVelocitiesAfterCollision(FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle, SpatialVector* firstVelocity, SpatialVector* secondVelocity) const
    {
        SpatialVector firstToSecondUnitVector;
        FillDifference(currentTime, firstParticle, secondParticle, &firstToSecondUnitVector);
        FLOAT_TYPE length = VectorUtilities::GetLength(firstToSecondUnitVector);
        VectorUtilities::DivideByValue(firstToSecondUnitVector, length, &firstToSecondUnitVector);

        // Parallel means parallel to the firstToSecondUnitVector
        SpatialVector firstParallelVelocity;
        SpatialVector secondParallelVelocity;

        SpatialVector firstTransverseVelocity;
        SpatialVector secondTransverseVelocity;

        FLOAT_TYPE firstParallelVelocityLength = SplitVectorIntoComponents(firstParticle.velocity, firstToSecondUnitVector, &firstParallelVelocity, &firstTransverseVelocity);
        FLOAT_TYPE secondParallelVelocityLength = SplitVectorIntoComponents(secondParticle.velocity, firstToSecondUnitVector, &secondParallelVelocity, &secondTransverseVelocity);

        FLOAT_TYPE radiusGrowthSum = (firstParticle.diameter + secondParticle.diameter) * ratioGrowthRate * 0.5;

        // We may choose boundary velocities such that their sum equals to radiusGrowthSum (so that even if both parallel velocities are zero, the particles will not collide due to radii increase).
        // But we choose twice as large boundary velocities to be very sure (in the original code by Donev each velocity was 2.0 * radiusGrowthSum).
        SpatialVector firstBoundaryVelocity;
        SpatialVector secondBoundaryVelocity;
        VectorUtilities::MultiplyByValue(firstToSecondUnitVector, 2.0 * radiusGrowthSum, &firstBoundaryVelocity);
        VectorUtilities::MultiplyByValue(firstToSecondUnitVector, - 2.0 * radiusGrowthSum, &secondBoundaryVelocity); // As it grows in an opposite direction

        // Collision means preserving transverse velocities and exchanging parallel velocities. Plus we should add the boundary velocities from neighbors.
        *firstVelocity = firstTransverseVelocity;
        VectorUtilities::Add(*firstVelocity, secondBoundaryVelocity, firstVelocity);
        VectorUtilities::Add(*firstVelocity, secondParallelVelocity, firstVelocity);

        *secondVelocity = secondTransverseVelocity;
        VectorUtilities::Add(*secondVelocity, firstParallelVelocity, secondVelocity);
        VectorUtilities::Add(*secondVelocity, firstBoundaryVelocity, secondVelocity);

        // See Otsuki and Hayakawa (2012) Critical scaling of a jammed system after a quench of temperature, formula (3)
        // (but the kinetic energy term should be multiplied by 2 to make p = nkT for ideal gas, as E = 3 / 2 nkT).
        // exchangedMomentum = sum by all events r_ij m_i delta_v_i (see also LubachevsckyStillingerStep::CalculateStatistics), computed NOT for the pair of particles in the event.
        // Do not include radiusGrowthSum to calculate pressure as if particles were not growing at all (as done in monosized LS code by Donev and Skoge).
        FLOAT_TYPE exchangedMomentum = (firstParallelVelocityLength + secondParallelVelocityLength) * length;
        return exchangedMomentum;
    }

    FLOAT_TYPE ParticleCollisionService::SplitVectorIntoComponents(const SpatialVector& vector, const SpatialVector& normal, SpatialVector* parallelToNormal, SpatialVector* transverseToNormal) const
    {
        FLOAT_TYPE parallelVectorProjection = VectorUtilities::GetDotProduct(normal, vector);
        VectorUtilities::MultiplyByValue(normal, parallelVectorProjection, parallelToNormal);
        VectorUtilities::Subtract(vector, *parallelToNormal, transverseToNormal);

        return std::abs(parallelVectorProjection);
    }

    FLOAT_TYPE ParticleCollisionService::GetCollisionTime(FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle) const
    {
        FLOAT_TYPE currentInnerDiameterRatio = initialInnerDiameterRatio + ratioGrowthRate * currentTime;

        SpatialVector relativeVelocity;
        VectorUtilities::Subtract(secondParticle.velocity, firstParticle.velocity, &relativeVelocity);
        FLOAT_TYPE relativeVelocitySquare = VectorUtilities::GetSelfDotProduct(relativeVelocity);

        SpatialVector difference;
        FillDifference(currentTime, firstParticle, secondParticle, &difference);
        FLOAT_TYPE distanceSquare = VectorUtilities::GetSelfDotProduct(difference);

        // Components of the quadratic equation a t t + 2 b t + c = 0. NOTE: a classical formula is a t t + b t + c = 0.
        FLOAT_TYPE radiiSumSquare = (firstParticle.diameter + secondParticle.diameter) * (firstParticle.diameter + secondParticle.diameter) * 0.25;
        FLOAT_TYPE a = relativeVelocitySquare - radiiSumSquare * ratioGrowthRate * ratioGrowthRate;
        // b is divided here by 2, and appropriate changes to discriminant and roots formulas are made. b is a relative sphere boundary velocity.
        FLOAT_TYPE b = VectorUtilities::GetDotProduct(relativeVelocity, difference) - radiiSumSquare * currentInnerDiameterRatio * ratioGrowthRate;
        FLOAT_TYPE c = distanceSquare - radiiSumSquare * currentInnerDiameterRatio * currentInnerDiameterRatio;

        return GetCollisionTime(currentTime, a, b, c);
    }

    FLOAT_TYPE ParticleCollisionService::GetCollisionTime(FLOAT_TYPE currentTime, FLOAT_TYPE a, FLOAT_TYPE b, FLOAT_TYPE c) const
    {
        // Spheres are overlapping
        if (c < 0.0)
        {
            // Boundaries are approaching or stationary
            if (b <= 0.0)
            {
//                printf("WARNING: Spheres already overlapping and approaching at GetCollisionTime. Overlap: %g, relative velocity: %g\n", c, b);
                return currentTime;
            }
            else
            {
                // Spheres are overlapping but boundaries are moving from each other
                return -1;
            }
        }
        // Spheres are in contact
        else if (c == 0)
        {
            // Boundaries are approaching
            if (b < 0.0)
            {
                return currentTime;
            }
            else
            {
                return -1;
            }
        }
        // Spheres are apart, c > 0
        else
        {
            // 2 b t + c == 0
            if (a == 0)
            {
                // Boundaries are approaching
                if (b < 0.0)
                {
                    return currentTime - c / b * 0.5;
                }
                // Handle the case b == 0
                else
                {
                    return -1;
                }
            }

            FLOAT_TYPE discriminant = b * b - a * c;

            if (discriminant < 0.0)
            {
                // Discriminant can be very small for double roots
                if (discriminant > - 10.0 * EPSILON)
                {
                    discriminant = 0.0;
                }
                // Will never collide
                else
                {
                    return -1;
                }
            }

            // Boundaries are either approaching each other (b <= 0.0) or diameters are growing faster than the spheres move from each other (a < 0.0).
            if (b <= 0.0 || a < 0.0)
            {
                FLOAT_TYPE discriminantSqrt = sqrt(discriminant);
                FLOAT_TYPE collisionTime = (- b - discriminantSqrt) / a;
                return currentTime + collisionTime;
            }
            else
            {
                return -1;
            }
        }
    }

    void ParticleCollisionService::FillDifference(FLOAT_TYPE currentTime, const MovingParticle& firstParticle, const MovingParticle& secondParticle, SpatialVector* difference) const
    {
        SpatialVector firstCoordinates;
        SpatialVector secondCoordinates;

        SpatialVector displacement;
        VectorUtilities::MultiplyByValue(firstParticle.velocity, currentTime - firstParticle.lastEventTime, &displacement);
        VectorUtilities::Add(firstParticle.coordinates, displacement, &firstCoordinates);

        VectorUtilities::MultiplyByValue(secondParticle.velocity, currentTime - secondParticle.lastEventTime, &displacement);
        VectorUtilities::Add(secondParticle.coordinates, displacement, &secondCoordinates);

        // difference is a vector from firstParticle to secondParticle = r2 - r1. It's consistent with relativeVelocity = v2 - v1
        mathService->FillDistance(secondCoordinates, firstCoordinates, difference);
    }
}

