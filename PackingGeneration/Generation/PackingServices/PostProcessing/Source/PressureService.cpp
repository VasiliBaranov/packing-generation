// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PressureService.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    PressureService::PressureService(MathService* mathService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {
    }

    void PressureService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    void PressureService::FillPressures(const vector<FLOAT_TYPE>& contractionRatios, const vector<FLOAT_TYPE>& energyPowers, vector<FLOAT_TYPE>* pressures) const
    {
        pressures->clear();

        int contractionRatiosCount = contractionRatios.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            FLOAT_TYPE contractionRatio = contractionRatios[i];
            FLOAT_TYPE energyPower = energyPowers[i];
            FLOAT_TYPE pressure = GetPressure(contractionRatio, energyPower);
            pressures->push_back(pressure);
        }
    }

    FLOAT_TYPE PressureService::GetPressure(FLOAT_TYPE contractionRatio, FLOAT_TYPE energyPower) const
    {
        FLOAT_TYPE pressureTensor[DIMENSIONS][DIMENSIONS];
        FillPressureTensor(contractionRatio, energyPower, pressureTensor);

        FLOAT_TYPE pressure = VectorUtilities::GetTrace<FLOAT_TYPE[DIMENSIONS][DIMENSIONS], FLOAT_TYPE>(pressureTensor, DIMENSIONS) / DIMENSIONS;
        return pressure;
    }

    FLOAT_TYPE PressureService::GetBulkModulus() const
    {
        const FLOAT_TYPE secondContractionRate = 0.9999;
        const FLOAT_TYPE firstContractionRate = 0.999;

        FLOAT_TYPE pressureDerivative = (GetPressure(secondContractionRate, 2.0) - GetPressure(firstContractionRate, 2.0)) / (secondContractionRate - firstContractionRate);
        FLOAT_TYPE bulkModulus = - 1.0 / DIMENSIONS * firstContractionRate * pressureDerivative;
        return bulkModulus;
    }

    void PressureService::FillPressureTensor(FLOAT_TYPE contractionRatio, FLOAT_TYPE energyPower, FLOAT_TYPE pressureTensor[DIMENSIONS][DIMENSIONS]) const
    {
        VectorUtilities::InitializeWith(pressureTensor, 0.0, DIMENSIONS, DIMENSIONS);
        const Packing& particlesRef = *particles;

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                const DomainParticle& neighbor = particlesRef[neighborIndex];

                UpdatePressureTensor(contractionRatio, energyPower, particle, neighbor, pressureTensor);
            }
        }

        FLOAT_TYPE packingVolume = VectorUtilities::GetProduct(config->packingSize) * pow(contractionRatio, DIMENSIONS);
        // Divide by 2 in contrast to the original paper, as they iterate over distinct particle pairs,
        // and we do over all pairs, taking symmetric pairs twice (though we use nearest neighbors lists, so it's much faster).
        FLOAT_TYPE normalizationFactor = - 1.0 / packingVolume / 2.0;
        VectorUtilities::Multiply(pressureTensor, normalizationFactor, pressureTensor, DIMENSIONS, DIMENSIONS);
    }

    void PressureService::UpdatePressureTensor(FLOAT_TYPE contractionRatio, FLOAT_TYPE energyPower, const DomainParticle& particle, const DomainParticle& neighbor, FLOAT_TYPE pressureTensor[DIMENSIONS][DIMENSIONS]) const
    {
        SpatialVector difference;
        SpatialVector direction;
        FLOAT_TYPE distance;

        FLOAT_TYPE normalizedDistance = mathService->GetNormalizedDistance(particle, neighbor);

        FLOAT_TYPE contractedNormalizedDistance = normalizedDistance * contractionRatio;
        if (contractedNormalizedDistance > 1.0)
        {
            return;
        }

        mathService->FillDistance(neighbor.coordinates, particle.coordinates, &difference);
        distance = normalizedDistance * (particle.diameter + neighbor.diameter) * 0.5; // or VectorUtilities::GetLength(difference);
        VectorUtilities::DivideByValue(difference, distance, &direction);
        // As if particles are contracted and overlapping, giving rise to the potential energy
        distance *= contractionRatio;

        for (int firstPressureDimension = 0; firstPressureDimension < DIMENSIONS; ++firstPressureDimension)
        {
            for (int secondPressureDimension = 0; secondPressureDimension < DIMENSIONS; ++secondPressureDimension)
            {
                FLOAT_TYPE pressureEntry = GetPressureEntry(particle, neighbor, firstPressureDimension, secondPressureDimension, direction, distance, energyPower);
                pressureTensor[firstPressureDimension][secondPressureDimension] += pressureEntry;
            }
        }
    }

    // See O�Hern et al (2003) The epitome of disorder.
    FLOAT_TYPE PressureService::GetPressureEntry(const DomainParticle& particle, const DomainParticle& neighbor,
            int firstPressureDimension, int secondPressureDimension,
            const SpatialVector& direction, FLOAT_TYPE distance, FLOAT_TYPE energyPower) const
    {
        const FLOAT_TYPE potentialNormalizer = 1.0;

        // If the potential energy is potentialNormalizer * (1.0 - distance / halfDiameter) ^ energyPower / energyPower
        FLOAT_TYPE halfDiameter = (particle.diameter + neighbor.diameter) * 0.5;
        FLOAT_TYPE potentialFirstDerivative = - potentialNormalizer * pow(1.0 - distance / halfDiameter, energyPower - 1.0) / halfDiameter;
        FLOAT_TYPE pressureEntry = distance * direction[firstPressureDimension] * direction[secondPressureDimension] * potentialFirstDerivative;

        return pressureEntry;
    }
}

