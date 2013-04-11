// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/DistanceService.h"
#include <complex>
#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Math.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "../Headers/INeighborProvider.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    DistanceService::DistanceService(MathService* mathService, GeometryService* geometryService, INeighborProvider* neighborProvider) :
            BaseDistanceService(mathService, neighborProvider)
    {
        this->geometryService = geometryService;
    }

    void DistanceService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    void DistanceService::StartMove(ParticleIndex particleIndex)
    {
        neighborProvider->StartMove(particleIndex);
    }

    void DistanceService::EndMove()
    {
        neighborProvider->EndMove();
    }

    FLOAT_TYPE DistanceService::GetDistanceToNearestSurface(const SpatialVector& point) const
    {
        const Packing& particlesRef = *particles;
        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(point, &neighborsCount);

        FLOAT_TYPE minDistance = MAX_FLOAT_VALUE;
        FLOAT_TYPE distance;

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            const DomainParticle* neighbor = &particlesRef[neighborIndexes[i]];
            distance = mathService->GetDistanceLength(point, neighbor->coordinates) - neighbor->diameter * 0.5;

            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }

        return minDistance;
    }

    FLOAT_TYPE DistanceService::GetDistanceToNearestSurface(ParticleIndex particleIndex) const
    {
        const Packing& particlesRef = *particles;
        // Calling GetNeighborIndexes for a particle is much more efficient for Verlet neighbor provider, than calling it for a point
        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);

        FLOAT_TYPE minDistance = MAX_FLOAT_VALUE;
        FLOAT_TYPE distance;

        const Particle* particle = &particlesRef[particleIndex];

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            const Particle* neighbor = &particlesRef[neighborIndexes[i]];
            distance = mathService->GetDistanceLength(particle->coordinates, neighbor->coordinates) - neighbor->diameter * 0.5;

            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }

        return minDistance;
    }

    ParticlePair DistanceService::FindClosestPair() const
    {
        ParticlePair pair;
        pair.normalizedDistanceSquare = MAX_FLOAT_VALUE;
        FLOAT_TYPE currentDistanceSquare = 0;

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                currentDistanceSquare = mathService->GetNormalizedDistanceSquare(particleIndex, neighborIndex, *particles);
                if (currentDistanceSquare < pair.normalizedDistanceSquare)
                {
                    pair.firstParticleIndex = particleIndex;
                    pair.secondParticleIndex = neighborIndex;
                    pair.normalizedDistanceSquare = currentDistanceSquare;
                }
            }
        }

        return pair;
    }

    ParticlePair DistanceService::FindClosestNeighbor(ParticleIndex particleIndex) const
    {
        FLOAT_TYPE currentDistanceSquare = 0;
        ParticlePair pair;
        pair.firstParticleIndex = particleIndex;
        pair.normalizedDistanceSquare = MAX_FLOAT_VALUE;

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];
            currentDistanceSquare = mathService->GetNormalizedDistanceSquare(particleIndex, neighborIndex, *particles);
            if (currentDistanceSquare < pair.normalizedDistanceSquare)
            {
                pair.secondParticleIndex = neighborIndex;
                pair.normalizedDistanceSquare = currentDistanceSquare;
            }
        }

        return pair;
    }

    void DistanceService::SetClosestNeighbor(const SpatialVector& point, ParticleIndex indexToExclude, FLOAT_TYPE diameter, ParticleIndex* neighborIndex, FLOAT_TYPE* normalizedDistanceSquare) const
    {
        const Packing& particlesRef = *particles;
        FLOAT_TYPE& normalizedDistanceSquareRef = *normalizedDistanceSquare;
        normalizedDistanceSquareRef = MAX_FLOAT_VALUE;
        FLOAT_TYPE currentDistanceSquare = 0;

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(point, &neighborsCount);
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex currentNeighborIndex = neighborIndexes[i];

            if (currentNeighborIndex == indexToExclude)
            {
                continue;
            }

            const DomainParticle* neighbor = &particlesRef[currentNeighborIndex];
            currentDistanceSquare = mathService->GetDistanceSquare(point, neighbor->coordinates) * 4.0 / (diameter + neighbor->diameter) / (diameter + neighbor->diameter);
            if (currentDistanceSquare < normalizedDistanceSquareRef)
            {
                *neighborIndex = currentNeighborIndex;
                normalizedDistanceSquareRef = currentDistanceSquare;
            }
        }
    }

    void DistanceService::FillPairCorrelationFunction(PairCorrelationFunction* pairCorrelationFunction) const
    {
        const Packing& particlesRef = *particles;
        FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particlesRef);
        FLOAT_TYPE minBinLeftEdge = 0.0;
        FLOAT_TYPE maxBinRightEdge = 6.0 * meanDiameter;
        FLOAT_TYPE step = 0.001;
        int binsCount = 0;
        Math::CalculateStepSize(minBinLeftEdge, maxBinRightEdge, step, &step, &binsCount);

        pairCorrelationFunction->binLeftEdges.resize(binsCount, 0.0);
        pairCorrelationFunction->binParticleCounts.resize(binsCount, 0);
        pairCorrelationFunction->pairCorrelationFunctionValues.resize(binsCount, 0.0);

        VectorUtilities::FillLinearScale(minBinLeftEdge, maxBinRightEdge - step, &pairCorrelationFunction->binLeftEdges);

        for (ParticleIndex i = 0; i < config->particlesCount - 1; ++i)
        {
            for (ParticleIndex j = i + 1; j < config->particlesCount; ++j)
            {
                FLOAT_TYPE distance = mathService->GetDistanceLength(particlesRef[i].coordinates, particlesRef[j].coordinates);
                int binIndex = static_cast<int>(std::floor(distance / step));
                if (binIndex >= binsCount)
                {
                    continue;
                }
                pairCorrelationFunction->binParticleCounts[binIndex]++;
            }
        }

        for (int binIndex = 0; binIndex < binsCount; ++binIndex)
        {
            // We consider just unique particle pairs when filling binParticleCounts. To convert to "all pairs mode" we need to multiply by 2.
            int particlesCount = 2 * pairCorrelationFunction->binParticleCounts[binIndex];
            pairCorrelationFunction->binParticleCounts[binIndex] = particlesCount;

            FLOAT_TYPE distance = pairCorrelationFunction->binLeftEdges[binIndex] + step * 0.5;
            FLOAT_TYPE sphereSurface = 4.0 * PI * distance * distance;
            pairCorrelationFunction->pairCorrelationFunctionValues[binIndex] = particlesCount / sphereSurface / step / config->particlesCount;
        }
    }

    // See http://en.wikipedia.org/wiki/Structure_factor, cf. (4) or (5).
    void DistanceService::FillStructureFactor(StructureFactor* structureFactor) const
    {
        // I'm using the equation from "Perfect Crystals" section, as it is faster, though may have worse precision (as doesn't use all the info from the particles).
        // It is also used in Jiao and Torquato (2011) Maximally random jammed packings of platonic solids, cf. 3.
        // For this code also Xu and Ching (2010) Effects of particle-size ratio on jamming of binary mixtures at zero temperature
        // "Due to the periodic boundary conditions, the wave vector must be chosen as..."
        structureFactor->waveVectorLengths.clear();
        structureFactor->structureFactorValues.clear();

        vector<StructureFactorPair> result;

        const int acceptableVectorsCount = 70000; // determined empirically
        const int maxDiscreteCoordinate = 12;
        const int currentVectorsCount = (2 * maxDiscreteCoordinate + 1) * (2 * maxDiscreteCoordinate + 1) * (2 * maxDiscreteCoordinate + 1);
        const FLOAT_TYPE vectorAcceptanceProbability = static_cast<FLOAT_TYPE>(acceptableVectorsCount) / currentVectorsCount;
        for (int i = -maxDiscreteCoordinate; i <= maxDiscreteCoordinate; ++i)
        {
            for (int j = -maxDiscreteCoordinate; j <= maxDiscreteCoordinate; ++j)
            {
                for (int k = -maxDiscreteCoordinate; k <= maxDiscreteCoordinate; ++k)
                {
                    if (i == 0 && j == 0 && k == 0)
                    {
                        continue;
                    }

                    SpatialVector waveVector;
                    waveVector[Axis::X] = i * 2.0 * PI / config->packingSize[Axis::X];
                    waveVector[Axis::Y] = j * 2.0 * PI / config->packingSize[Axis::Y];
                    waveVector[Axis::Z] = k * 2.0 * PI / config->packingSize[Axis::Z];

                    FLOAT_TYPE waveVectorLength = VectorUtilities::GetLength(waveVector);

                    // Usually the first maximum of structure factor is around 2 * PI
                    if (waveVectorLength > 2.0 * PI)
                    {
                        FLOAT_TYPE randomVariable = Math::GetNextRandom();
                        if (randomVariable > vectorAcceptanceProbability)
                        {
                            continue;
                        }
                    }

                    complex<FLOAT_TYPE> complexSum(0.0, 0.0);
                    complex<FLOAT_TYPE> imaginaryUnit(0.0, 1.0);
                    for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
                    {
                        const Packing& particlesRef = *particles;
                        FLOAT_TYPE dotProduct = VectorUtilities::GetDotProduct(waveVector, particlesRef[particleIndex].coordinates);
                        complexSum += exp(imaginaryUnit * dotProduct);
                    }

                    StructureFactorPair pair;
                    pair.waveVectorLength = waveVectorLength;
                    pair.structureFactorValue = norm(complexSum) / config->particlesCount;
                    result.push_back(pair);
                }
            }
        }

        StlUtilities::Sort(&result);

        for (vector<StructureFactorPair>::const_iterator it = result.begin(); it != result.end(); ++it)
        {
            StructureFactorPair pair = *it;

            structureFactor->waveVectorLengths.push_back(pair.waveVectorLength);
            structureFactor->structureFactorValues.push_back(pair.structureFactorValue);
        }
    }
}

