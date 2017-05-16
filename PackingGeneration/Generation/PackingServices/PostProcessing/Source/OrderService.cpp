// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/OrderService.h"

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/StlUtilities.h"
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
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("Order service only supports 3D");
        }

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

        const Packing& particlesRef = *particles;
        for (int particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];
            if (particlesRef[particleIndex].isImmobile) // exclude fixed particles or particles outside the active geometry
            {
                continue;
            }

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
        const Packing& particlesRef = *particles;
        for (int particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];
            if (particlesRef[particleIndex].isImmobile) // exclude fixed particles or particles outside the active geometry
            {
                continue;
            }

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

    void OrderService::FillLocalOrientationalDisorder(LocalOrientationalDisorder* localOrientationalDisorder) const
    {
        // See Bargiel, Tory (2001) Packing fraction and measures of disorder of ultradense irregular packings of equal spheres. II. Transition from dense random packing
        const FLOAT_TYPE fccAngles[] = {60.0, 90.0, 120.0, 180.0};
        const int fccAnglesCounts[] = {24, 12, 24, 6};
        ReferenceLattice fccLattice("fcc", 12, fccAngles, fccAnglesCounts, 4);

        const FLOAT_TYPE hcpAngles[] = {60.0, 90.0, 120.0, 180.0, 109.47, 146.44};
        const int hcpAnglesCounts[] = {24, 12, 18, 3, 3, 6};
        ReferenceLattice hcpLattice("hcp", 12, hcpAngles, hcpAnglesCounts, 6);

        const FLOAT_TYPE icoAngles[] = {63.4349, 116.5651, 180.0};
        const int icoAnglesCounts[] = {30, 30, 6};
        ReferenceLattice icoLattice("ico", 12, icoAngles, icoAnglesCounts, 3);

        vector<const ReferenceLattice*> referenceLattices;
        referenceLattices.push_back(&fccLattice);
        referenceLattices.push_back(&hcpLattice);
        referenceLattices.push_back(&icoLattice);

        FillLocalOrientationalDisorder(referenceLattices, localOrientationalDisorder);
    }

    // TODO: refactor!!!
    void OrderService::FillLocalOrientationalDisorder(const vector<const ReferenceLattice*>& referenceLattices, LocalOrientationalDisorder* localOrientationalDisorder) const
    {
        int latticeVectorsCount = referenceLattices[0]->latticeVectorsCount;
        localOrientationalDisorder->referenceLatticeNames.clear();
        for (size_t i = 0; i < referenceLattices.size(); ++i)
        {
            localOrientationalDisorder->referenceLatticeNames.push_back(referenceLattices[i]->name);
            if (referenceLattices[i]->latticeVectorsCount != latticeVectorsCount)
            {
                throw NotImplementedException("Currently support lattices of equal size.");
            }
        }

        localOrientationalDisorder->disordersPerParticle.resize(config->particlesCount);
        localOrientationalDisorder->closeNeighborsPerParticle.resize(config->particlesCount);

        const Packing& particlesRef = *particles;
        vector<SpatialVector> directions;
        vector<FLOAT_TYPE> distances;
        vector<int> sortingPermutation;

        int anglesCount = latticeVectorsCount * (latticeVectorsCount - 1) / 2;
        vector<FLOAT_TYPE> angles(anglesCount);
        vector<FLOAT_TYPE> angleDifferences(anglesCount);

        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];

            // Fill approximate neighbors

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            directions.resize(neighborsCount);
            distances.resize(neighborsCount);

            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                const DomainParticle& neighbor = particlesRef[neighborIndex];

                mathService->FillDistance(neighbor.coordinates, particle.coordinates, &directions[i]);
                distances[i] = VectorUtilities::GetLength(directions[i]);
            }

            // Find the indexes of 12 nearest neighbors (without exact sorting)
            StlUtilities::FindNthElementPermutation(distances, latticeVectorsCount - 1, &sortingPermutation);

            // Normalize directions for the closest neighbors
            for (int i = 0; i < latticeVectorsCount; ++i)
            {
                int directionIndex = sortingPermutation[i];
                VectorUtilities::DivideByValue(directions[directionIndex], distances[directionIndex], &directions[directionIndex]);
            }

            // Store closest neighbor indexes
            vector<ParticleIndex>& closeNeighborIndexes = localOrientationalDisorder->closeNeighborsPerParticle[particleIndex];
            closeNeighborIndexes.resize(latticeVectorsCount);
            for (int i = 0; i < latticeVectorsCount; ++i)
            {
                closeNeighborIndexes[i] = neighborIndexes[sortingPermutation[i]];
            }
            StlUtilities::Sort(&closeNeighborIndexes);

            // Find all the angles between directions to the closest neighbors
            int angleIndex = 0;
            for (int i = 0; i < latticeVectorsCount - 1; ++i)
            {
                int firstNeighborIndex = sortingPermutation[i];
                SpatialVector& firstDirection = directions[firstNeighborIndex];

                for (int j = i + 1; j < latticeVectorsCount; ++j)
                {
                    int secondNeighborIndex = sortingPermutation[j];
                    SpatialVector& secondDirection = directions[secondNeighborIndex];

                    FLOAT_TYPE dotProduct = VectorUtilities::GetDotProduct(firstDirection, secondDirection);
                    if (dotProduct < -1)
                    {
                        dotProduct = -1;
                    }
                    if (dotProduct > 1)
                    {
                      dotProduct = 1;
                    }
                    FLOAT_TYPE angle = std::abs(acos(dotProduct));
                    angles[angleIndex] = angle;
                    angleIndex = angleIndex + 1;
                }
            }

            StlUtilities::Sort(&angles);

            // Compare angles to the angles from reference lattices
            vector<FLOAT_TYPE>& currentDisorders = localOrientationalDisorder->disordersPerParticle[particleIndex];
            currentDisorders.resize(referenceLattices.size());
            for (size_t i = 0; i < referenceLattices.size(); ++i)
            {
                const vector<FLOAT_TYPE>& referenceAngles = referenceLattices[i]->sortedAngles;
                VectorUtilities::Subtract(angles, referenceAngles, &angleDifferences);
                FLOAT_TYPE disorder = VectorUtilities::GetLength(angleDifferences);
                disorder /= std::sqrt(static_cast<FLOAT_TYPE>(angleDifferences.size()));

                currentDisorders[i] = disorder;
            }
        }
    }

    OrderService::ReferenceLattice::ReferenceLattice(string name, int latticeVectorsCount, const Core::FLOAT_TYPE uniqueAnglesInDegrees[], const int uniqueAnglesCounts[], int arraySize)
    {
        this->name = name;
        this->latticeVectorsCount = latticeVectorsCount;

        for (int angleIndex = 0; angleIndex < arraySize; ++angleIndex)
        {
            for (int i = 0; i < uniqueAnglesCounts[angleIndex]; ++i)
            {
                sortedAngles.push_back(uniqueAnglesInDegrees[angleIndex] * PI / 180.0);
            }
        }

        StlUtilities::Sort(&sortedAngles);
    }
}

