// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT
#include "../Headers/DistanceService.h"

#include <cstdio>
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

    void DistanceService::FillDistancesToClosestSurfaces(const Core::SpatialVector& point, vector<FLOAT_TYPE>* distancesToClosestSurfaces) const
    {
        const Packing& particlesRef = *particles;
        vector<FLOAT_TYPE>& distancesToClosestSurfacesRef = *distancesToClosestSurfaces;

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(point, &neighborsCount);

//        int expectedDistancesCount = distancesToClosestSurfacesRef.size();
        distancesToClosestSurfacesRef.resize(neighborsCount);

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            const DomainParticle* neighbor = &particlesRef[neighborIndexes[i]];
            FLOAT_TYPE distance = mathService->GetDistanceLength(point, neighbor->coordinates) - neighbor->diameter * 0.5;
            distancesToClosestSurfacesRef[i] = distance;
        }

//        if (expectedDistancesCount < neighborsCount)
//        {
//            distancesToClosestSurfacesRef.resize(expectedDistancesCount);
//        }
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
            FLOAT_TYPE sphereSurface;
            if (DIMENSIONS == 3)
            {
                sphereSurface = 4.0 * PI * distance * distance;
            }
            if (DIMENSIONS == 2)
            {
                sphereSurface = 2.0 * PI * distance;
            }
            pairCorrelationFunction->pairCorrelationFunctionValues[binIndex] = particlesCount / sphereSurface / step / config->particlesCount;
        }
    }

    // See http://en.wikipedia.org/wiki/Structure_factor, cf. (4) or (5).
    // TODO: Make static or move to another service!
    void DistanceService::FillStructureFactor(const std::vector<ParticleIndex>& particleIndexesOfInterest, StructureFactor* structureFactor) const
    {
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("2D structure factor not implemented");
        }

        vector<Core::SpatialVector> periodicWaveVectors;
        vector<Core::FLOAT_TYPE> waveVectorLengths;
        FillPeriodicWaveVectorsUpToPeak(*config, &periodicWaveVectors, &waveVectorLengths);

        vector<FLOAT_TYPE> structureFactorValues;
        FillStructureFactorForWaveVectors(*config, *particles, particleIndexesOfInterest, periodicWaveVectors, &structureFactorValues);

        vector<int> permutation;
        StlUtilities::SortPermutation(waveVectorLengths, &permutation);

        structureFactor->waveVectorLengths.resize(waveVectorLengths.size());
        structureFactor->structureFactorValues.resize(waveVectorLengths.size());
        for (size_t i = 0; i < waveVectorLengths.size(); ++i)
        {
            structureFactor->waveVectorLengths[i] = waveVectorLengths[permutation[i]];
            structureFactor->structureFactorValues[i] = structureFactorValues[permutation[i]];
        }
    }

    void DistanceService::FillPeriodicWaveVectorsUpToPeak(const SystemConfig& config,
                            vector<SpatialVector>* periodicWaveVectors, vector<FLOAT_TYPE>* waveVectorLengths) const
    {
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("2D structure factor not implemented");
        }

        // I'm using the equation from "Perfect Crystals" section, as it is faster, though may have worse precision (as doesn't use all the info from the particles).
        // It is also used in Jiao and Torquato (2011) Maximally random jammed packings of platonic solids, cf. 3.
        // For this code see also Xu and Ching (2010) Effects of particle-size ratio on jamming of binary mixtures at zero temperature
        // "Due to the periodic boundary conditions, the wave vector must be chosen as..."
        vector<SpatialVector>& periodicWaveVectorsRef = *periodicWaveVectors;
        vector<FLOAT_TYPE>& waveVectorLengthsRef = *waveVectorLengths;

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
                    waveVector[Axis::X] = i * 2.0 * PI / config.packingSize[Axis::X];
                    waveVector[Axis::Y] = j * 2.0 * PI / config.packingSize[Axis::Y];
                    waveVector[Axis::Z] = k * 2.0 * PI / config.packingSize[Axis::Z];

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

                    periodicWaveVectorsRef.push_back(waveVector);
                    waveVectorLengthsRef.push_back(waveVectorLength);
                }
            }
        }
    }

    void DistanceService::FillPeriodicWaveVectors(const SystemConfig& config, FLOAT_TYPE expectedWaveVectorLength, FLOAT_TYPE waveVectorHalfWidth,
            vector<SpatialVector>* periodicWaveVectors, vector<FLOAT_TYPE>* waveVectorLengths) const
    {
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("2D structure factor not implemented");
        }

        // I'm using the equation from "Perfect Crystals" section, as it is faster, though may have worse precision (as doesn't use all the info from the particles).
        // It is also used in Jiao and Torquato (2011) Maximally random jammed packings of platonic solids, cf. 3.
        // For this code see also Xu and Ching (2010) Effects of particle-size ratio on jamming of binary mixtures at zero temperature
        // "Due to the periodic boundary conditions, the wave vector must be chosen as..."

        vector<SpatialVector>& periodicWaveVectorsRef = *periodicWaveVectors;
        vector<FLOAT_TYPE>& waveVectorLengthsRef = *waveVectorLengths;

        periodicWaveVectorsRef.clear();
        waveVectorLengthsRef.clear();

        FLOAT_TYPE maxPackingSize = VectorUtilities::GetMaxValue(config.packingSize);
        int maxDiscreteCoordinate = std::ceil((expectedWaveVectorLength + waveVectorHalfWidth) * maxPackingSize / (2.0 * PI));

        // Scattering function is symmetrical with respect to wave vectors. Can leave only half of them. That's why start at zero in the first loop.
        // Can't start at one, because for zero values there may be significant wave vectors
        for (int i = 0; i <= maxDiscreteCoordinate; ++i)
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
                    waveVector[Axis::X] = i * 2.0 * PI / config.packingSize[Axis::X];
                    waveVector[Axis::Y] = j * 2.0 * PI / config.packingSize[Axis::Y];
                    waveVector[Axis::Z] = k * 2.0 * PI / config.packingSize[Axis::Z];

                    FLOAT_TYPE waveVectorLength = VectorUtilities::GetLength(waveVector);

                    bool vectorHasCorrectLength = (waveVectorLength >= expectedWaveVectorLength - waveVectorHalfWidth) && (waveVectorLength <= expectedWaveVectorLength + waveVectorHalfWidth);
                    if (!vectorHasCorrectLength)
                    {
                        continue;
                    }

                    // Can't omit this check, because in the YZ plane (with X == 0) there still may be several symmetrical wave vectors
                    SpatialVector reflectedWaveVector;
                    VectorUtilities::MultiplyByValue(waveVector, -1, &reflectedWaveVector);
                    bool symmetricVectorExists = StlUtilities::Exists(periodicWaveVectorsRef, reflectedWaveVector);
                    if (symmetricVectorExists)
                    {
                        continue;
                    }

                    periodicWaveVectorsRef.push_back(waveVector);
                    waveVectorLengthsRef.push_back(waveVectorLength);
                }
            }
        }
    }

    void DistanceService::FillStructureFactorForWaveVectors(
        const SystemConfig& config, 
        const Packing& particles,
        const std::vector<ParticleIndex>& particleIndexesOfInterest,
        const vector<SpatialVector>& waveVectors, 
        vector<FLOAT_TYPE>* structureFactors) const
    {
        vector<FLOAT_TYPE> selfPartValues; // TODO: support passing null and omitting the self-part computation!
        FillIntermediateScatteringFunctionForWaveVectors(
            config, 
            particles, 
            particles, 
            particleIndexesOfInterest, 
            waveVectors, 
            structureFactors, 
            &selfPartValues);
    }

    void DistanceService::FillIntermediateScatteringFunctionForWaveVectors(
        const SystemConfig& config, 
        const Packing& firstPacking, 
        const Packing& secondPacking,
        const std::vector<ParticleIndex>& particleIndexesOfInterest,
        const vector<SpatialVector>& waveVectors, 
        vector<FLOAT_TYPE>* intermediateScatteringFunctionValues, 
        vector<FLOAT_TYPE>* selfPartValues) const
    {
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("2D structure factor not implemented");
        }

        vector<FLOAT_TYPE>& intermediateScatteringFunctionValuesRef = *intermediateScatteringFunctionValues;
        vector<FLOAT_TYPE>& selfPartValuesRef = *selfPartValues;
        intermediateScatteringFunctionValuesRef.resize(waveVectors.size());
        selfPartValuesRef.resize(waveVectors.size());

        // TODO: rewrite
        ParticleIndex particlesOfInterestCount = config.particlesCount;
        if (particleIndexesOfInterest.size() > 0)
        {
            particlesOfInterestCount = particleIndexesOfInterest.size();
        }

        for (size_t i = 0; i < waveVectors.size(); ++i)
        {
            const SpatialVector& waveVector = waveVectors[i];
            // Can't compute structure factor with the logic as in Perez-Angel, et al, 2011, Equilibration of concentrated hard-sphere fluids; Berthier, Witten, 2009 (this is the self-part ISF)
            // Use logic as everywhere else: van Megen, Underwood, 1994; Martinez et al 2014; Williams, van Megen, 2001; Pusey, 2008 (full ISF)
            // I wanted to use this function for computing structure factor as well, so implemented the full ISF.
            // Disadvantage: no known fit form. For the self-part--stretched exponent (Kohlrausch form).
            complex<FLOAT_TYPE> firstComplexSum(0.0, 0.0);
            complex<FLOAT_TYPE> secondComplexSum(0.0, 0.0);
            complex<FLOAT_TYPE> selfPartValue(0.0, 0.0);
            complex<FLOAT_TYPE> imaginaryUnit(0.0, 1.0);

            for (ParticleIndex i = 0; i < particlesOfInterestCount; ++i)
            {
                ParticleIndex particleIndex;
                if (particleIndexesOfInterest.size() == 0)
                {
                    particleIndex = i;
                }
                else
                {
                    particleIndex = particleIndexesOfInterest[i];
                }

                FLOAT_TYPE firstDotProduct = VectorUtilities::GetDotProduct(waveVector, firstPacking[particleIndex].coordinates);
                complex<FLOAT_TYPE> firstExponent = exp(imaginaryUnit * firstDotProduct);
                firstComplexSum += firstExponent;

                if (&firstPacking != &secondPacking)
                {
                    FLOAT_TYPE secondDotProduct = VectorUtilities::GetDotProduct(waveVector, secondPacking[particleIndex].coordinates);
                    complex<FLOAT_TYPE> secondExponent = exp(imaginaryUnit * secondDotProduct);
                    secondComplexSum += secondExponent;

                    // exp(imaginaryUnit * (firstDotProduct - secondDotProduct)) = exp(imaginaryUnit * waveVector.*(firstCoordinates - secondCoordinates))
                    complex<FLOAT_TYPE> secondExponentConjugate = std::conj(secondExponent); //complex<FLOAT_TYPE> secondExponentConjugate(secondExponent.real(), -secondExponent.imag());
                    selfPartValue += (firstExponent * secondExponentConjugate);

                    // NOTE: i don't select best periodic images, because the discrete Fourier transform assumes infinite repeated system.
                    // In other words, my wave vectors are constructed in such a way that adding any Lx*n*ex + Ly*m*ey+Lz*k*ez to particle coordinates will not change complex sums,
                    // because wave vectors have the form 2 pi * (ex*p/Lx + ey*p/Ly + ez*q/Lz).
                }
            }

            if (&firstPacking == &secondPacking)
            {
                secondComplexSum = firstComplexSum;

                selfPartValue = particlesOfInterestCount;
            }

            complex<FLOAT_TYPE> secondComplexSumConjugate = std::conj(secondComplexSum);
            FLOAT_TYPE intermediateScatteringFunctionValue = (firstComplexSum * secondComplexSumConjugate).real(); // In case both sums were initially equal, the result is the norm (squared length)
            intermediateScatteringFunctionValuesRef[i] = intermediateScatteringFunctionValue / particlesOfInterestCount; // If the first and second packings are equal, returns structure factor
            selfPartValuesRef[i] = selfPartValue.real() / particlesOfInterestCount;
        }
    }

    void DistanceService::FillNeighborVectorSums(vector<FLOAT_TYPE>* neighborVectorSumsNorms, FLOAT_TYPE contractionRate) const
    {
        const Packing& particlesRef = *particles;
        vector<FLOAT_TYPE>& neighborVectorSumsNormsRef = *neighborVectorSumsNorms;
        neighborVectorSumsNormsRef.resize(config->particlesCount);
        VectorUtilities::InitializeWith(neighborVectorSumsNorms, 0.0);
        int nonRattlersCount = 0;
        int contactsCount = 0;

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle* particle = &particlesRef[particleIndex];
            SpatialVector neighborVectorsSum;
            VectorUtilities::InitializeWith(&neighborVectorsSum, 0.0);

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);

            bool hasNeighbors = false;
            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex currentNeighborIndex = neighborIndexes[i];
                const DomainParticle* neighbor = &particlesRef[currentNeighborIndex];

                SpatialVector neighborVector;
                mathService->FillDistance(particle->coordinates, neighbor->coordinates, &neighborVector);
                FLOAT_TYPE distanceSquare = VectorUtilities::GetSelfDotProduct(neighborVector);

                FLOAT_TYPE normalizedDistanceSquare = distanceSquare * 4.0 / (particle->diameter + neighbor->diameter) / (particle->diameter + neighbor->diameter);
                FLOAT_TYPE contractedNormalizedDistanceSquare = contractionRate * contractionRate * normalizedDistanceSquare;
                if (contractedNormalizedDistanceSquare <= 1.0)
                {
                    VectorUtilities::Add(neighborVectorsSum, neighborVector, &neighborVectorsSum);
                    hasNeighbors = true;
                    contactsCount++;
                }
            }

            if (hasNeighbors)
            {
                nonRattlersCount++;
            }

            neighborVectorSumsNormsRef[particleIndex] = VectorUtilities::GetLength(neighborVectorsSum);
        }

        FLOAT_TYPE coordinationNumber = static_cast<FLOAT_TYPE>(contactsCount) / nonRattlersCount;
        printf("Coordination number estimate: %g\n", coordinationNumber);
    }

    FLOAT_TYPE DistanceService::GetMaxNeighborVectorSumForNonRattlers(const vector<bool>& rattlerMask, const vector<FLOAT_TYPE>& neighborVectorSumsNorms) const
    {
        FLOAT_TYPE maxNeighborVectorSumNorm = 0.0;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            if (!rattlerMask[particleIndex] && neighborVectorSumsNorms[particleIndex] > maxNeighborVectorSumNorm)
            {
                maxNeighborVectorSumNorm = neighborVectorSumsNorms[particleIndex];
            }
        }

        return maxNeighborVectorSumNorm;
    }

    void DistanceService::FillClosestPairs(vector<ParticlePair>* closestPairs) const
    {
        vector<ParticlePair>& closestPairsRef = *closestPairs;

        closestPairsRef.resize(config->particlesCount);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            ParticlePair closestPair = FindClosestNeighbor(particleIndex);
            closestPairsRef[particleIndex] = closestPair;
        }
    }
}

