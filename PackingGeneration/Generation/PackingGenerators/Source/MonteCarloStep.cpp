// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MonteCarloStep.h"
#include <stdio.h>
#include "Core/Headers/Constants.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Math.h"
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/Geometries/Headers/IGeometry.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    MonteCarloStep::MonteCarloStep(GeometryService* geometryService,
            DistanceService* distanceService,
            MathService* mathService) :
            BasePackingStep(geometryService, NULL, NULL, mathService)
    {
        this->distanceService = distanceService;
        initialExpansionFactor = 2.0;
        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = false;
    }

    MonteCarloStep::~MonteCarloStep()
    {

    }

    void MonteCarloStep::SetContext(const ModellingContext& context)
    {
        BasePackingStep::SetContext(context);
        distanceService->SetContext(context);
    }

    void MonteCarloStep::SetParticles(Packing* particles)
    {
        BasePackingStep::SetParticles(particles);
        distanceService->SetParticles(*particles);
        isFirstRun = true;

        maxDisplacementToFreeLengthRatio = 2.0;

        ParticlePair closestPair = distanceService->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);

        UpdateParameters(closestPair);
     }

    void MonteCarloStep::DisplaceParticles()
    {
        if (isFirstRun)
        {
            isFirstRun = false;

            ArrangeInCubicArray();
            distanceService->SetParticles(*particles);
            ParticlePair closestPair = distanceService->FindClosestPair();

            innerDiameterRatio = 1.0 / initialExpansionFactor; // Should continue will automatically become true
            UpdateParameters(closestPair);
        }
        else
        {
            ParticlePair closestPair;
            bool success = false;
            while (!success)
            {
                closestPair = DisplaceParticlesOneTime(&success);
                if (!success)
                {
                    printf("None of the particles was moved. Reducing max displacement length.");
                    const FLOAT_TYPE reductionRate = 0.5;
                    maxDisplacementLength *= reductionRate;
                }
            }

            UpdateParameters(closestPair);
        }
    }

    ParticlePair MonteCarloStep::DisplaceParticlesOneTime(bool* success)
    {
        Packing& particlesRef = *particles;
        FLOAT_TYPE minNormalizedDistanceSquare = MAX_FLOAT_VALUE;
        ParticlePair closestPair;

        ParticleIndex successfulMovesCount = 0;
        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            DomainParticle clone = particlesRef[i];
            DomainParticle& particle = particlesRef[i];

            DisplaceParticle(&clone);
            ParticleIndex neighborIndex;
            FLOAT_TYPE normalizedDistanceSquare;
            distanceService->SetClosestNeighbor(clone.coordinates, clone.index, clone.diameter, &neighborIndex, &normalizedDistanceSquare);

            // Particles do not overlap, if their diameter is reduced by diameterRatio
            if (normalizedDistanceSquare > innerDiameterRatio * innerDiameterRatio)
            {
                geometry->EnsureBoundaries(particle, &clone, minNormalizedDistance);

                distanceService->StartMove(i);
                particle.coordinates = clone.coordinates;
                distanceService->EndMove();

                if (minNormalizedDistanceSquare > normalizedDistanceSquare)
                {
                    closestPair.firstParticleIndex = i;
                    closestPair.secondParticleIndex = neighborIndex;
                    closestPair.normalizedDistanceSquare = normalizedDistanceSquare;
                }
                successfulMovesCount++;
            }
        }

        FLOAT_TYPE successfulMovesRatio = static_cast<FLOAT_TYPE>(successfulMovesCount) / static_cast<FLOAT_TYPE>(config->particlesCount);
        if (successfulMovesRatio < 0.5)
        {
            maxDisplacementToFreeLengthRatio *= 0.75;
            printf("Successful moves ratio %e is too low. Updating maxDisplacementToFreeLengthRatio to %e\n", successfulMovesRatio, maxDisplacementToFreeLengthRatio);
        }

        *success = (successfulMovesCount > 0);
        return closestPair;
    }

    void MonteCarloStep::DisplaceParticle(DomainParticle* particle)
    {
        FLOAT_TYPE factor = maxDisplacementLength / RAND_MAX;
        SpatialVector displacement;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            displacement[i] = factor * rand();
        }

        VectorUtilities::Add(particle->coordinates, displacement, &particle->coordinates);
    }

    void MonteCarloStep::UpdateParameters(ParticlePair closestPair)
    {
        Packing& particlesRef = *particles;
        //Update global min distance
        DomainParticle& closestParticle = particlesRef[closestPair.firstParticleIndex];
        minNormalizedDistance = sqrt(closestPair.normalizedDistanceSquare);

        FLOAT_TYPE radiiSum = (closestParticle.diameter + particlesRef[closestPair.secondParticleIndex].diameter) * 0.5;
        FLOAT_TYPE minDistance = radiiSum * minNormalizedDistance;

        //Update diameterRatio
        //See Maier (2008) Sensitivity of pore-scale dispersion to the construction of random bead packs
        //Expansion factor is used in the original article (i.e. the box is expanded). We use diameterRatio instead, as if particles were contracted. diameterRatio = 1 / expansionFactor
        //No overlaps will be induced by this update. Substitute contractionRate = 1, then diameterRatio = minDistance / radiiSum. I.e. closest pair starts to touch each other.
        innerDiameterRatio *= (radiiSum + generationConfig->contractionRate * (minDistance / innerDiameterRatio - radiiSum)) / radiiSum;

        //Update maxDisplacementLength
        FLOAT_TYPE occupiedLength = radiiSum * innerDiameterRatio; //determines the length along the diameter centers, occupied by particles, if particles were decreased by diameter ratio.
        FLOAT_TYPE availableLength = minDistance - occupiedLength;

        //availableLength > 0 before diameterRatio update, as particle move is allowed just if there is no intersection of reduced particles.
        //availableLength > 0 even after diameterRatio update, as diameterRatio is updated to ensure that no overlaps are induced (see above).
        maxDisplacementLength = maxDisplacementToFreeLengthRatio * availableLength;
    }

    void MonteCarloStep::ArrangeInCubicArray()
    {
        Packing& particlesRef = *particles;
        ParticleIndex particleCountByOneSide = static_cast<ParticleIndex>(Math::Round(pow(config->particlesCount, 1.0 / 3.0)));
        SpatialVector cellSize;

        VectorUtilities::DivideByValue(config->packingSize, static_cast<FLOAT_TYPE>(particleCountByOneSide), &cellSize);

        // Other particles remain unchanged.
        for (ParticleIndex i = 0; i < particleCountByOneSide; ++i)
        {
            for (ParticleIndex j = 0; j < particleCountByOneSide; ++j)
            {
                for (ParticleIndex k = 0; k < particleCountByOneSide; ++k)
                {
                    ParticleIndex index = k * particleCountByOneSide * particleCountByOneSide + j * particleCountByOneSide + i;
                    DomainParticle& particle = particlesRef[index];

                    SpatialVector displacement;
                    displacement[Axis::X] = 0.5 + i;
                    displacement[Axis::Y] = 0.5 + j;
                    displacement[Axis::Z] = 0.5 + k;

                    VectorUtilities::Multiply(cellSize, displacement, &particle.coordinates);
                }
            }
        }
    }

    void MonteCarloStep::ResetGeneration()
    {

    }
}

