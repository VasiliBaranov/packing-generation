// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/OrderService.h"

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/SphericalHarmonicsComputer.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    OrderService::OrderService(MathService* mathService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {

    }

    void OrderService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    void OrderService::FillParticleDirections(vector<NeighborDirections>* particleDirections) const
    {
        SpatialVector difference;
        NeighborDirection neighborDirection;

        const Packing& particlesRef = *particles;
        vector<NeighborDirections>& particleDirectionsRef = *particleDirections;
        particleDirectionsRef.resize(config->particlesCount);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];
            NeighborDirections& currentParticleDirections = particleDirectionsRef[particleIndex];

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                const DomainParticle& neighbor = particlesRef[neighborIndex];

                mathService->FillDistance(neighbor.coordinates, particle.coordinates, &difference);
                FLOAT_TYPE distance = VectorUtilities::GetLength(difference);
    //                if (distance > 1.2 * particle->diameter)
                if (distance > 0.6 * (particle.diameter + neighbor.diameter))
                {
                    continue;
                }

                VectorUtilities::DivideByValue(difference, distance, &neighborDirection.direction);
                neighborDirection.neighborIndex = neighborIndex;
                currentParticleDirections.push_back(neighborDirection); //copy the structure here; neighborDirection.direction is also copied as it's compile-time array, stored inside the structure
            }
        }
    }

    OrderService::Order OrderService::GetOrder(int l) const
    {
        vector<NeighborDirections> particleDirections;
        FillParticleDirections(&particleDirections);

        Order result;
        result.globalOrder = GetGlobalOrder(particleDirections, l);
        result.localOrder = GetLocalOrder(particleDirections, l);
        return result;
    }

    FLOAT_TYPE OrderService::GetGlobalOrder(const vector<NeighborDirections>& particleDirections, int l) const
    {
        int sphericalHarmonicsCount = 2 * l + 1;
        vector<complex<FLOAT_TYPE> > currentHarmonicValues(sphericalHarmonicsCount);
        vector<complex<FLOAT_TYPE> > harmonicValues(sphericalHarmonicsCount);
        int directionsCount = 0;

        VectorUtilities::InitializeWith(&harmonicValues, 0.0);

        for (int particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const vector<OrderService::NeighborDirection>& neighborDirections = particleDirections[particleIndex];
            directionsCount += neighborDirections.size();

            for (vector<OrderService::NeighborDirection>::const_iterator directionsIterator = neighborDirections.begin(); directionsIterator != neighborDirections.end(); ++directionsIterator)
            {
                // Do not want a copy constructor of vector<OrderService::DynamicSpatialVector> to be called when dereferencing the iterator, therefore assign to reference
                const OrderService::NeighborDirection& neighborDirection = *directionsIterator;

                SphericalHarmonicsComputer::FillSphericalHarmonicValues(neighborDirection.direction, l, &currentHarmonicValues);
                VectorUtilities::Add(currentHarmonicValues, harmonicValues, &harmonicValues);
            }
        }

        FLOAT_TYPE orderSquaresSum = 0.0;
        VectorUtilities::DivideByValue(harmonicValues, static_cast<FLOAT_TYPE>(directionsCount), &harmonicValues);
        for (int i = 0; i < sphericalHarmonicsCount; ++i)
        {
            orderSquaresSum += norm(harmonicValues[i]);
        }

        return sqrt(4.0 * PI / sphericalHarmonicsCount * orderSquaresSum);
    }

    FLOAT_TYPE OrderService::GetLocalOrder(const vector<NeighborDirections>& particleDirections, int l) const
    {
        FLOAT_TYPE meanLocalOrder = 0.0;
        int validParticlesCount = 0;
        for (int particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const vector<OrderService::NeighborDirection>& neighborDirections = particleDirections[particleIndex];

            FLOAT_TYPE particleOrder = GetParticleOrder(neighborDirections, l);
            if (particleOrder >= 0)
            {
                meanLocalOrder += particleOrder;
                validParticlesCount++;
            }
        }

        meanLocalOrder /= validParticlesCount;
        return meanLocalOrder;
    }

    FLOAT_TYPE OrderService::GetParticleOrder(const vector<NeighborDirection>& neighborDirections, int l) const
    {
        if (neighborDirections.size() == 0)
        {
            return -1.0;
        }

        int sphericalHarmonicsCount = 2 * l + 1;
        vector<complex<FLOAT_TYPE> > currentHarmonicValues(sphericalHarmonicsCount);
        vector<complex<FLOAT_TYPE> > harmonicValues(sphericalHarmonicsCount);

        VectorUtilities::InitializeWith(&harmonicValues, 0.0);

        for (vector<OrderService::NeighborDirection>::const_iterator directionsIterator = neighborDirections.begin(); directionsIterator != neighborDirections.end(); ++directionsIterator)
        {
            // Do not want a copy constructor of vector<OrderService::DynamicSpatialVector> to be called when dereferencing the iterator, therefore assign to reference
            const OrderService::NeighborDirection& neighborDirection = *directionsIterator;

            SphericalHarmonicsComputer::FillSphericalHarmonicValues(neighborDirection.direction, l, &currentHarmonicValues);
            VectorUtilities::Add(currentHarmonicValues, harmonicValues, &harmonicValues);
        }

        FLOAT_TYPE orderSquaresSum = 0.0;
        VectorUtilities::DivideByValue(harmonicValues, static_cast<FLOAT_TYPE>(neighborDirections.size()), &harmonicValues);
        for (int i = 0; i < sphericalHarmonicsCount; ++i)
        {
            orderSquaresSum += norm(harmonicValues[i]);
        }

        return sqrt(4.0 * PI / sphericalHarmonicsCount * orderSquaresSum);
    }
}

