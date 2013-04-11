// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MathService.h"

#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    MathService::MathService()
    {

    }

    void MathService::SetContext(const ModellingContext& context)
    {
        config = context.config;
        VectorUtilities::MultiplyByValue(config->packingSize, 0.5, &packingHalfSize);
    }

    FLOAT_TYPE MathService::GetNormalizedDistanceSquare(ParticleIndex n1, ParticleIndex n2, const Packing& particles) const
    {
        return GetNormalizedDistanceSquare(particles[n1], particles[n2]);
    }

    FLOAT_TYPE MathService::GetNormalizedDistance(const Particle& firstParticle, const Particle& secondParticle) const
    {
        return sqrt(GetNormalizedDistanceSquare(firstParticle, secondParticle));
    }

    FLOAT_TYPE MathService::GetNormalizedDistanceSquare(const Particle& firstParticle, const Particle& secondParticle) const
    {
        FLOAT_TYPE distanceSquare = GetDistanceSquare(firstParticle.coordinates, secondParticle.coordinates);
        FLOAT_TYPE diametersSquare = (firstParticle.diameter + secondParticle.diameter) * (firstParticle.diameter + secondParticle.diameter);

        return distanceSquare * 4.0 / diametersSquare;
    }

    FLOAT_TYPE MathService::GetDistanceLength(const SpatialVector& firstPoint, const SpatialVector& secondPoint) const
    {
        FLOAT_TYPE distanceSquare = GetDistanceSquare(firstPoint, secondPoint);
        return sqrt(distanceSquare);
    }

    FLOAT_TYPE MathService::GetDistanceSquare(const SpatialVector& firstPoint, const SpatialVector& secondPoint) const
    {
        SpatialVector difference;
        FillDistance(firstPoint, secondPoint, &difference);

        return VectorUtilities::GetSelfDotProduct(difference);
    }

    void MathService::FillDistance(const SpatialVector& to, const SpatialVector& from, SpatialVector* difference) const
    {
        // When we subtract "2" from "1", the vector is from "1" to "2"
        VectorUtilities::Subtract(to, from, difference);
        ReflectPeriodically(difference, DIMENSIONS - 1);

        // TODO: Move to IGeometry class
        if (config->boundariesMode == BoundariesMode::Bulk)
        {
            for (int i = 0; i < DIMENSIONS - 1; ++i)
            {
                ReflectPeriodically(difference, i);
            }
        }
    }

    void MathService::FillClosestPeriodicImagePosition(const SpatialVector& stablePoint, const SpatialVector& movablePoint, SpatialVector* periodicImage) const
    {
        SpatialVector difference;
        FillDistance(stablePoint, movablePoint, &difference);
        VectorUtilities::Subtract(stablePoint, difference, periodicImage);
    }

    void MathService::ReflectPeriodically(SpatialVector* distance, int dimension) const
    {
        SpatialVector& distanceRef = *distance;

        // Should change the sign of the vector element when reflecting the particles periodically
        if (distanceRef[dimension] > packingHalfSize[dimension])
        {
            distanceRef[dimension] -= config->packingSize[dimension];
        }
        if (distanceRef[dimension] < -packingHalfSize[dimension])
        {
            distanceRef[dimension] += config->packingSize[dimension];
        }
    }
}

