// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VerletListNeighborProvider.h"

#include "Core/Headers/StlUtilities.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    const FLOAT_TYPE VerletListNeighborProvider::MAX_EXPECTED_OUTER_DIAMETER_RATIO = 1.1;

    VerletListNeighborProvider::VerletListNeighborProvider(GeometryService* geometryService, MathService* mathService, INeighborProvider* baseNeighborProvider, GeometryCollisionService* geometryCollisionService)
    {
        this->geometryService = geometryService;
        this->baseNeighborProvider = baseNeighborProvider;
        this->mathService = mathService;
        this->geometryCollisionService = geometryCollisionService;

        cutoffDistanceSet = false;
    }

    void VerletListNeighborProvider::SetContext(const ModellingContext& context)
    {
        config = context.config;
        geometryService->SetContext(context);
        baseNeighborProvider->SetContext(context);
    }

    VerletListNeighborProvider::~VerletListNeighborProvider()
    {
    }

    void VerletListNeighborProvider::SetCutoffDistance(Core::FLOAT_TYPE cutoffDistance)
    {
        this->cutoffDistance = cutoffDistance;
        cutoffDistanceSet = true;
    }

    void VerletListNeighborProvider::SetParticles(const Packing& particles)
    {
        this->particles = &particles;

        if (!cutoffDistanceSet)
        {
            cutoffDistance = geometryService->GetMeanParticleDiameter(particles);
    //        cutoffDistance = 0.5 * geometryService->GetMinParticleDiameter(particles);
    //        cutoffDistance = 0.5 * geometryService->GetMeanParticleDiameter(particles);
        }

        verletParticles.clear();
        verletLists.clear();
        verletParticles.resize(config->particlesCount);
        verletLists.resize(config->particlesCount);

        Particle::CopyPackingTo(particles, &verletParticles);
        for (int i = 0; i < config->particlesCount; ++i)
        {
            verletParticles[i].diameter = verletParticles[i].diameter * MAX_EXPECTED_OUTER_DIAMETER_RATIO + cutoffDistance;
            verletLists[i].reserve(50);
        }

        baseNeighborProvider->SetParticles(verletParticles);

        SpreadParticlesByLists();
    }

    void VerletListNeighborProvider::SpreadParticlesByLists()
    {
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            FillVerletList(particleIndex);
        }
    }

    void VerletListNeighborProvider::FillVerletList(ParticleIndex particleIndex)
    {
        // Should not clear the list:
        // 1. it is already prefilled with some of the neighbors from processing of previous particles
        // 2. more importantly, in case if baseNeighborProvider has bugs and returns asymmetric neighbors, it will lead to asymmetric neighbors in VerletListNeighborProvider.
        // In turn, it will lead to incorrect Lubachevsky-Stillinger event search operation, some events will be missed.
        // vector<ParticleIndex>* verletList = &verletLists[particleIndex];
        // verletList->clear();

        ParticleIndex neighborsCount;
        const ParticleIndex* neighborIndexes = baseNeighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);

        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = neighborIndexes[i];

            if (IsParticleInVerletSphere(particleIndex, neighborIndex))
            {
                // NOTE: currently we add particles and check for duplicates while adding.
                // It may be more efficient to add without checking, than in the next cycle sort and remove duplicates.
                AddParticleToVerletList(particleIndex, neighborIndex);

                // Don't forget to update the neighbour's Verlet list, too
                AddParticleToVerletList(neighborIndex, particleIndex);
            }
        }
    }

    bool VerletListNeighborProvider::IsParticleInVerletSphere(ParticleIndex verletListIndex, ParticleIndex particleIndex) const
    {
        const Particle& particle = verletParticles[verletListIndex];
        const Particle& neighbor = verletParticles[particleIndex];

//        FLOAT_TYPE distanceSquare = mathService->GetDistanceSquare(particle->coordinates, neighbor->coordinates);
//        FLOAT_TYPE maxDistance = cutoffDistance + particle->diameter * 0.5 * MAX_EXPECTED_OUTER_DIAMETER_RATIO + neighbor->diameter * 0.5 * MAX_EXPECTED_OUTER_DIAMETER_RATIO;
//        return distanceSquare <= maxDistance * maxDistance;

        // The code above is equivalent, since we update verletParticles diameters to include cutoffDistance (see SetParticles).
        FLOAT_TYPE normalizedDistanceSquare = mathService->GetNormalizedDistanceSquare(particle, neighbor);
        return normalizedDistanceSquare <= 1.0;
    }

    void VerletListNeighborProvider::AddParticleToVerletList(ParticleIndex verletListIndex, ParticleIndex particleIndex)
    {
        vector<ParticleIndex>& verletList = verletLists[verletListIndex];
        if (!StlUtilities::Exists(verletList, particleIndex))
        {
            verletList.push_back(particleIndex);
        }
    }

    void VerletListNeighborProvider::RemoveParticleFromVerletList(ParticleIndex verletListIndex, ParticleIndex particleIndex)
    {
        vector<ParticleIndex>* verletList = &verletLists[verletListIndex];
        StlUtilities::QuicklyRemove(verletList, particleIndex);
    }

    const ParticleIndex* VerletListNeighborProvider::GetNeighborIndexes(ParticleIndex particleIndex, ParticleIndex* neighborsCount) const
    {
        cutoffDistanceSet = false;
        const vector<ParticleIndex>& verletList = verletLists[particleIndex];
        *neighborsCount = verletList.size();
        return &verletList[0];
    }

    const ParticleIndex* VerletListNeighborProvider::GetNeighborIndexes(const SpatialVector& coordinates, ParticleIndex* neighborsCount) const
    {
        cutoffDistanceSet = false;
        return baseNeighborProvider->GetNeighborIndexes(coordinates, neighborsCount);
    }

    void VerletListNeighborProvider::EndMove()
    {
        const Packing& particlesRef = *particles;
        const DomainParticle* particle = &particlesRef[movedParticleIndex];
        DomainParticle* verletParticle = &verletParticles[movedParticleIndex];

        FLOAT_TYPE shiftSquare = mathService->GetDistanceSquare(particle->coordinates, verletParticle->coordinates);

        bool shouldUpdate = shiftSquare > 0.25 * cutoffDistance * cutoffDistance;
        if (!shouldUpdate)
        {
            return;
        }

        baseNeighborProvider->StartMove(movedParticleIndex);
        verletParticle->coordinates = particle->coordinates;
        baseNeighborProvider->EndMove();

        // Remove the moved particle from all the previous neighbors' Verlet lists
        vector<ParticleIndex>& verletList = verletLists[movedParticleIndex];
        int neighborsCount = verletList.size();
        for (ParticleIndex i = 0; i < neighborsCount; ++i)
        {
            ParticleIndex neighborIndex = verletList[i];
            RemoveParticleFromVerletList(neighborIndex, movedParticleIndex);
        }

        FillVerletList(movedParticleIndex);
    }

    void VerletListNeighborProvider::StartMove(ParticleIndex particleIndex)
    {
        movedParticleIndex = particleIndex;
    }

    FLOAT_TYPE VerletListNeighborProvider::GetTimeToUpdateBoundary(ParticleIndex particleIndex, const SpatialVector& point, const SpatialVector& velocity) const
    {
        FLOAT_TYPE verletSphereRadius = 0.5 * cutoffDistance;
        SpatialVector pointPeriodicImage;
        mathService->FillClosestPeriodicImagePosition(verletParticles[particleIndex].coordinates, point, &pointPeriodicImage);
        FLOAT_TYPE collisionTime = geometryCollisionService->GetSphereIntersectionTime(pointPeriodicImage, velocity, verletParticles[particleIndex].coordinates, verletSphereRadius);
        return collisionTime;
    }
}

