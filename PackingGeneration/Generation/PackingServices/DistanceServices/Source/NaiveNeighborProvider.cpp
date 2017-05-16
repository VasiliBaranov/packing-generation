// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/NaiveNeighborProvider.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/Model/Headers/Config.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    NaiveNeighborProvider::NaiveNeighborProvider(GeometryCollisionService* geometryCollisionService)
    {
        this->geometryCollisionService = geometryCollisionService;
    }

    void NaiveNeighborProvider::SetContext(const ModellingContext& context)
    {
        this->config = context.config;
        SpatialVector minVertexCoordinates;
        minVertexCoordinates.assign(0);
        box.Initialize(minVertexCoordinates, config->packingSize);

        allNeighborIndexes.clear();
        allNeighborIndexes.resize(config->particlesCount);

        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            allNeighborIndexes[i] = i;
        }
        lastUsedParticleIndex = config->particlesCount - 1;
    }

    void NaiveNeighborProvider::SetParticles(const DomainParticle& particles)
    {

    }

    const ParticleIndex* NaiveNeighborProvider::GetNeighborIndexes(ParticleIndex particleIndex, ParticleIndex* neighborsCount) const
    {
        ParticleIndex lastIndex = config->particlesCount - 1;

        // Restore the ordering of the indexes in the array
        allNeighborIndexes[lastIndex] = lastIndex;
        allNeighborIndexes[lastUsedParticleIndex] = lastUsedParticleIndex;

        // Move current particle index to the end to exclude from the returned neighbor indexes
        allNeighborIndexes[particleIndex] = lastIndex;
        allNeighborIndexes[lastIndex] = particleIndex;
        lastUsedParticleIndex = particleIndex;

        *neighborsCount = config->particlesCount - 1;
        return &allNeighborIndexes[0];
    }

    const ParticleIndex* NaiveNeighborProvider::GetNeighborIndexes(const SpatialVector& coordinates, ParticleIndex* neighborsCount) const
    {
        return &allNeighborIndexes[0];
    }

    void NaiveNeighborProvider::EndMove()
    {
        //Do nothing
    }

    void NaiveNeighborProvider::StartMove(ParticleIndex particleIndex)
    {
        //Do nothing
    }

    FLOAT_TYPE NaiveNeighborProvider::GetTimeToUpdateBoundary(ParticleIndex particleIndex, const SpatialVector& point, const SpatialVector& velocity) const
    {
        FLOAT_TYPE intersectionTime;
        int wallIndex;
        geometryCollisionService->FindIntersection(point, velocity, box, -1, &intersectionTime, &wallIndex);
        return intersectionTime;
    }

    NaiveNeighborProvider::~NaiveNeighborProvider()
    {
    }
}

