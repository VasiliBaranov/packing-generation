// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ImmobileParticlesService.h"

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Math.h"
#include "Core/Headers/Constants.h"
#include "Core/Geometry/Headers/GeometryParameters.h"
#include "Core/Lattice/Headers/ColumnMajorIndexingProvider.h"

#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/Geometries/Headers/IGeometry.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    ImmobileParticlesService::ImmobileParticlesService(MathService* mathService, GeometryService* geometryService, INeighborProvider* neighborProvider)
    {
        this->mathService = mathService;
        this->geometryService = geometryService;
        this->neighborProvider = neighborProvider;
    }

    ImmobileParticlesService::~ImmobileParticlesService()
    {

    }

    void ImmobileParticlesService::SetContext(const ModellingContext& context)
    {
        this->config = context.config;
        this->geometry = context.geometry;

        geometryService->SetContext(context);
        mathService->SetContext(context);
        neighborProvider->SetContext(context);
    }

    void ImmobileParticlesService::SetAndArrangeImmobileParticles(Packing* particles)
    {
        this->particles = particles;
        neighborProvider->SetParticles(*particles);

        // See Zaccarelli et al (2009) Crystallization of Hard-Sphere Glasses, Fig. 1
        FLOAT_TYPE particleDiameterStd = geometryService->GetParticleDiameterStd(*particles);
        if (particleDiameterStd >= 0.1)
        {
            return;
        }

        DiscreteSpatialVector cellsPerSide;
        FillCellsPerSide(&cellsPerSide);
        int cellsCount = VectorUtilities::GetProduct(cellsPerSide);

        Packing& particlesRef = *particles;
        for (int cellIndex = 0; cellIndex < cellsCount; ++cellIndex)
        {
            SpatialVector cellCenter;
            FillCellCenter(cellIndex, cellsPerSide, &cellCenter);

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(cellCenter, &neighborsCount);

            vector<FLOAT_TYPE> distancesToCellCenter;
            FillDistancesToCenter(neighborIndexes, neighborsCount, cellCenter, &distancesToCellCenter);

            int immobileNeighborLocalIndex = GetImmobileNeighborLocalIndex(distancesToCellCenter);
            ParticleIndex immobileParticleIndex = neighborIndexes[immobileNeighborLocalIndex];
            DomainParticle& particle = particlesRef[immobileParticleIndex];

            particle.isImmobile = true;
        }
    }

    void ImmobileParticlesService::FillCellsPerSide(DiscreteSpatialVector* cellsPerSide) const
    {
        // See Filion, Hermes, Ni, Dijkstra (2010) Crystal nucleation of hard spheres using molecular dynamics, umbrella sampling, and forward flux sampling: A comparison of simulation techniques; Fig. 7.
        // But for the value 50 boxes are so small that particles closest to their centers are too close to each other.
        const int clusterSize = 25;
        const int immobileParticlesPerCluster = 1;

        const FLOAT_TYPE particlesPerImmobileParticle = static_cast<FLOAT_TYPE>(clusterSize) / immobileParticlesPerCluster;
        const FLOAT_TYPE cellsCount = config->particlesCount / particlesPerImmobileParticle;
        int cellsPerSideCount = floor(std::pow(cellsCount, 1.0 / DIMENSIONS));
        VectorUtilities::InitializeWith(cellsPerSide, cellsPerSideCount);

        // Try to increase the size by each dimension by one, until the number of cells is the closest to the expected.
        // Increasing sizes by all axes by one is equivalent to cellsPerSideCount = ceil(std::pow(cellsCount, 1.0 / DIMENSIONS)),
        // and cellsPerSideCount^3 will clearly be larger than cellsCount. So we may try just first two dimensions.
        DiscreteSpatialVector& cellsPerSideRef = *cellsPerSide;
        for (int i = 0; i < DIMENSIONS - 1; ++i)
        {
            cellsPerSideRef[i]++;
            int actualCellsCount = VectorUtilities::GetProduct(cellsPerSideRef);

            if (actualCellsCount > cellsCount)
            {
                cellsPerSideRef[i]--;
                break;
            }
        }

//        // Ensure that particles around centers of the cells will allow the ideal glass configuration
//        FLOAT_TYPE cellSideMultiplier = 1.0 / cellsPerSide;
//        SpatialVector cellSize;
//        VectorUtilities::MultiplyByValue(config->packingSize, cellSideMultiplier, &cellSize);
//        int minIndex = StlUtilities::FindMinElementPosition(cellSize);
//        FLOAT_TYPE minCellSize = cellSize[minIndex];

//        // The glass close packing density for monodisperse particles. See Baranau and Tallarek (2013) Phys Rev E
//        const FLOAT_TYPE idealGlassDensity = 0.65;
//        // crystallineDensity = particlesCount * pi / 6 * maxParticleDiameter ^ 3 / totalVolume
//        // maxParticleDiameter ^ 3 = totalVolume * 6 * crystallineDensity / pi / particlesCount
//        FLOAT_TYPE maxParticleDiameter = std::pow(geometry->GetTotalVolume() * 6.0 * idealGlassDensity / PI / config->particlesCount, 1.0 / DIMENSIONS);
//        // Two diameters between cell centers per each axis.
//        // The correlation length between particles is ~4r (see pair correlation functions, e.g. Donev (2005)), so immobile particles will not interfere each other
//        const FLOAT_TYPE tolerance = 2.0;
//
//        if (minCellSize < maxParticleDiameter * tolerance)
//        {
//            minCellSize = maxParticleDiameter;
//            cellsPerSide = floor(config->packingSize[minIndex] / minCellSize);
//        }

//        return cellsPerSide;
    }

    void ImmobileParticlesService::FillCellCenter(int cellIndex, const DiscreteSpatialVector& cellsPerSide, SpatialVector* cellCenter) const
    {
        SpatialVector cellSize;
        VectorUtilities::Divide(config->packingSize, cellsPerSide, &cellSize);

        SpatialVector cellHalfSize;
        VectorUtilities::MultiplyByValue(cellSize, 0.5, &cellHalfSize);

        GeometryParameters geometryParameters;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            geometryParameters.SetSystemSize(i, cellsPerSide[i]);
            geometryParameters.SetPeriodicity(i, true);
        }

        ColumnMajorIndexingProvider linearIndexingProvider;
        linearIndexingProvider.SetGeometryParameters(&geometryParameters);

        DiscreteSpatialVector cellVector;
        linearIndexingProvider.FillMultidimensionalIndexes(cellIndex, &cellVector);

        VectorUtilities::Multiply(cellVector, cellSize, cellCenter);
        VectorUtilities::Add(*cellCenter, cellHalfSize, cellCenter);
    }

    int ImmobileParticlesService::GetImmobileNeighborLocalIndex(const vector<FLOAT_TYPE>& distancesToCellCenter) const
    {
        FLOAT_TYPE meanParticleRadius = 0.5 * geometryService->GetMeanParticleDiameter(*particles);
        vector<int> closestNeighborsPermutation;
        StlUtilities::SortPermutation(distancesToCellCenter, &closestNeighborsPermutation);

        int validNeighborsCount = distancesToCellCenter.size();
        for (size_t i = 0; i < distancesToCellCenter.size(); ++i)
        {
            int particleIndex = closestNeighborsPermutation[i];
            if (distancesToCellCenter[particleIndex] > meanParticleRadius)
            {
                validNeighborsCount = i;
                break;
            }
        }

        int randomParticleNearCellCenter = (validNeighborsCount == 0) ? 0 : std::rand() % validNeighborsCount;
        int immobileNeighborIndex = closestNeighborsPermutation[randomParticleNearCellCenter];
        return immobileNeighborIndex;
    }

    void ImmobileParticlesService::FillDistancesToCenter(const ParticleIndex* neighborIndexes, ParticleIndex neighborsCount, const SpatialVector& cellCenter, vector<FLOAT_TYPE>* distancesToCellCenter) const
    {
        vector<FLOAT_TYPE>& distancesToCellCenterRef = *distancesToCellCenter;
        distancesToCellCenterRef.resize(neighborsCount);

        const vector<DomainParticle>& particlesRef = *particles;
        for (ParticleIndex neighborIndex = 0; neighborIndex < neighborsCount; ++neighborIndex)
        {
            ParticleIndex particleIndex = neighborIndexes[neighborIndex];
            const Particle& particle = particlesRef[particleIndex];
            FLOAT_TYPE distanceToCellCenter = mathService->GetDistanceLength(particle.coordinates, cellCenter);
            distancesToCellCenterRef[neighborIndex] = distanceToCellCenter;
        }
    }

}

