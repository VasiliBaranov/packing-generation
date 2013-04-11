// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/InsertionRadiiGenerator.h"

#include <stdio.h>
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/Model/Headers/Config.h"
#include "Core/Headers/VectorUtilities.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    InsertionRadiiGenerator::InsertionRadiiGenerator(DistanceService* distanceProvider, GeometryService* geometryService)
    {
        this->distanceProvider = distanceProvider;
        this->geometryService = geometryService;
    }

    InsertionRadiiGenerator::~InsertionRadiiGenerator()
    {

    }

    void InsertionRadiiGenerator::SetContext(const ModellingContext& context)
    {
        geometry = context.geometry;
        config = context.config;
        distanceProvider->SetContext(context);
        geometryService->SetContext(context);
    }

    void InsertionRadiiGenerator::FillInsertionRadii(const Packing& particles, int insertionRadiiCount, vector<FLOAT_TYPE>* insertionRadii) const
    {
        distanceProvider->SetParticles(particles);
        insertionRadii->reserve(insertionRadiiCount);

        FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particles);

        for (int i = 0; i < insertionRadiiCount; ++i)
        {
            SpatialVector point;
            VectorUtilities::InitializeWithRandoms(&point);
            VectorUtilities::Multiply(point, config->packingSize, &point);

            FLOAT_TYPE insertionRadius = distanceProvider->GetDistanceToNearestSurface(point) / meanDiameter;
            insertionRadii->push_back(insertionRadius);
        }
    }

    // A faster implementation, no need to allocate insertion radii array (80 Mb usually)
    FLOAT_TYPE InsertionRadiiGenerator::GetLargePoresDensity(const Packing& particles, int insertionRadiiCount, FLOAT_TYPE minPoreRadius) const
    {
        distanceProvider->SetParticles(particles);
        FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particles);

        int largePoresCount = 0;
        for (int i = 0; i < insertionRadiiCount; ++i)
        {
            SpatialVector point;
            VectorUtilities::InitializeWithRandoms(&point);
            VectorUtilities::Multiply(point, config->packingSize, &point);

            FLOAT_TYPE poreRadius = distanceProvider->GetDistanceToNearestSurface(point) / meanDiameter;
            if (poreRadius >= minPoreRadius)
            {
                largePoresCount++;
            }
        }

        return largePoresCount;
    }

    FLOAT_TYPE InsertionRadiiGenerator::CalculateEntropy(const Packing& particles, int minInsertionRadiiCount) const
    {
        // Insertion radii are normalized by meanDiameter, which corresponds to rescaling the packing to make particles have mean radius = 0.5.
        // Therefore we can used a fixed samplePoreRadius and meanParticleRadius.
        const FLOAT_TYPE minPoreRadius = 0.25;
        const FLOAT_TYPE meanParticleRadius = 0.5;
        const FLOAT_TYPE normalizationFactor = (meanParticleRadius / minPoreRadius) * (meanParticleRadius / minPoreRadius);

        distanceProvider->SetParticles(particles);
        FLOAT_TYPE meanDiameter = geometryService->GetMeanParticleDiameter(particles);

        const int checkPeriod = 10000;
        const FLOAT_TYPE relativeTolerance = 1e-3; // entropy values -20 and -21 are acceptably close

        int insertionRadiiCount = 0;
        int largePoresCount = 0;
        FLOAT_TYPE entropy = 0;
        FLOAT_TYPE previousEntropy = 0;

        while (1)
        {
            if (insertionRadiiCount % checkPeriod == 0 && insertionRadiiCount > 0)
            {
                FLOAT_TYPE largePoresDensity = static_cast<FLOAT_TYPE>(largePoresCount) / insertionRadiiCount;
                entropy = log(largePoresDensity) * normalizationFactor;
                bool toleranceCorrect = std::abs((previousEntropy - entropy) / entropy) < relativeTolerance;
                if (toleranceCorrect && insertionRadiiCount >= minInsertionRadiiCount)
                {
                    break;
                }
                else
                {
                    previousEntropy = entropy;
                }
            }

            SpatialVector point;
            VectorUtilities::InitializeWithRandoms(&point);
            VectorUtilities::Multiply(point, config->packingSize, &point);

            FLOAT_TYPE poreRadius = distanceProvider->GetDistanceToNearestSurface(point) / meanDiameter;
            if (poreRadius > minPoreRadius)
            {
                largePoresCount++;
            }

            insertionRadiiCount++;
        }

        printf("insertionRadiiCount: %d\n", insertionRadiiCount);
        FLOAT_TYPE largePoresDensity = static_cast<FLOAT_TYPE>(largePoresCount) / insertionRadiiCount;
        entropy = log(largePoresDensity) * normalizationFactor;

        return entropy;
    }
}

