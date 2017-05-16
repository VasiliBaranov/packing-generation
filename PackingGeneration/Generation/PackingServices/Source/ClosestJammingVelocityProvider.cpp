// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ClosestJammingVelocityProvider.h"
#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Constants.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/BondsProvider.h"

using namespace PackingGenerators;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    ClosestJammingVelocityProvider::ClosestJammingVelocityProvider(MathService* mathService) : particleCollisionService(mathService)
    {
        this->mathService = mathService;

        // We measure time in inner diameter ratio units, so ratio growth rate is always 1.0.
        // We will use innerDiameterRatio as current time below, so put an initial ratio to zero.
        particleCollisionService.Initialize(0.0, 1.0);
    }

    ClosestJammingVelocityProvider::~ClosestJammingVelocityProvider()
    {

    }

    clock_t ClosestJammingVelocityProvider::FillVelocities(const INeighborProvider& neighborProvider,
            const BondsProvider& bondsProvider,
            const Model::Packing& particles,
            Core::FLOAT_TYPE innerDiameterRatio,
            std::vector<Core::SpatialVector>* particleVelocities)
    {
        this->neighborProvider = &neighborProvider;
        this->bondsProvider = &bondsProvider;
        this->particles = &particles;
        this->innerDiameterRatio = innerDiameterRatio;
        this->particleVelocities = particleVelocities;

        // Fill a system of linear equations for Lagrange multipliers
        int bondsCount = this->bondsProvider->GetBonds().size();

        Eigen::SparseMatrix<FLOAT_TYPE> optimizationMatrix(bondsCount, bondsCount);
        Eigen::VectorXd rightSide(bondsCount);

        FillOptimizationMatrix(&optimizationMatrix);
        FillOptimizationRightSide(&rightSide);
        // WriteLinearSystem(optimizationMatrix, rightSide);

        // Solve it. Other methods (SimplicialLLT and SimplicialLDLT) are very slow for large bond and bond pairs count (e.g. 20 000 bonds and 90 000 bond pairs)
        Eigen::ConjugateGradient<Eigen::SparseMatrix<FLOAT_TYPE> > linearSystemSolver;

        clock_t startTime = clock();
        linearSystemSolver.compute(optimizationMatrix);
        Eigen::VectorXd lagrangianMultipliers = linearSystemSolver.solve(rightSide);
        clock_t solutionTime = clock() - startTime;

        FillVelocities(lagrangianMultipliers);
        return solutionTime;
    }

//    void ClosestJammingVelocityProvider::WriteLinearSystem(const Eigen::SparseMatrix<FLOAT_TYPE>& optimizationMatrix, const Eigen::VectorXd& rightSide)
//    {
//        string matrixPath = Path::Append(generationConfig->baseFolder, "optimizationMatrix.txt");
//        ScopedFile<ExceptionErrorHandler> optimizationMatrixFile(matrixPath, "w");
//
//        for (int k = 0; k < optimizationMatrix.outerSize(); ++k)
//        {
//            for (Eigen::SparseMatrix<FLOAT_TYPE>::InnerIterator it(optimizationMatrix, k); it; ++it)
//            {
//                fprintf(optimizationMatrixFile, "%d %d %g\n", it.row(), it.col(), it.value());
//            }
//        }
//
//        string rightSidePath = Path::Append(generationConfig->baseFolder, "rightSide.txt");
//        ScopedFile<ExceptionErrorHandler> rightSideFile(rightSidePath, "w");
//
//        for (int i = 0; i < rightSide.rows(); ++i)
//        {
//            fprintf(rightSideFile, "%g\n", rightSide[i]);
//        }
//    }

    FLOAT_TYPE ClosestJammingVelocityProvider::FindBestMovementTime()
    {
        FLOAT_TYPE minCollisionTime = MAX_FLOAT_VALUE;

        for (size_t particleIndex = 0; particleIndex < particles->size(); ++particleIndex)
        {
            int neighborsCount;
            const int* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            MovingParticle particle = CreateMovingParticle(particleIndex);

            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];

                // Equations of movement ensure that particles that share a bond (equivalently, are in close contact) will never intersect.
                if (bondsProvider->ParticlesShareBond(particleIndex, neighborIndex))
                {
                    continue;
                }

                FLOAT_TYPE currentCollisionTime = particleCollisionService.GetCollisionTime(innerDiameterRatio, particle, CreateMovingParticle(neighborIndex));
                if (currentCollisionTime > innerDiameterRatio && currentCollisionTime < minCollisionTime)
                {
                    minCollisionTime = currentCollisionTime;
                }
            }
        }

        FLOAT_TYPE movementTime = minCollisionTime - innerDiameterRatio;
        return movementTime;
    }

    MovingParticle ClosestJammingVelocityProvider::CreateMovingParticle(ParticleIndex particleIndex)
    {
        const Model::Packing& particlesRef = *particles;
        const DomainParticle& particle = particlesRef[particleIndex];

        MovingParticle movingParticle;
        particle.CopyTo(&movingParticle);

        vector<SpatialVector>& particleVelocitiesRef = *particleVelocities;
        movingParticle.velocity = particleVelocitiesRef[particleIndex];
        movingParticle.lastEventTime = innerDiameterRatio;

        return movingParticle;
    }

    void ClosestJammingVelocityProvider::FillVelocities(const Eigen::VectorXd& lagrangeMultipliers)
    {
        const Model::Packing& particlesRef = *particles;
        vector<SpatialVector>& particleVelocitiesRef = *particleVelocities;
        for (size_t particleIndex = 0; particleIndex < particles->size(); ++particleIndex)
        {
            const DomainParticle& particle = particlesRef[particleIndex];
            const vector<int>& currentBondIndexes = bondsProvider->GetBondIndexesPerParticle()[particleIndex];

            SpatialVector& velocity = particleVelocitiesRef[particleIndex];
            VectorUtilities::InitializeWith(&velocity, 0.0);

            if (currentBondIndexes.size() == 0)
            {
                continue;
            }

            for (size_t i = 0; i < currentBondIndexes.size(); ++i)
            {
                int bondIndex = currentBondIndexes[i];

                ParticleIndex neighborIndex = bondsProvider->GetBonds()[bondIndex].GetOtherIndex(particleIndex);
                const DomainParticle& neighbor = particlesRef[neighborIndex];

                SpatialVector bondVector;
                mathService->FillDistance(particle.coordinates, neighbor.coordinates, &bondVector);
                FLOAT_TYPE bondLength = VectorUtilities::GetLength(bondVector);
                VectorUtilities::DivideByValue(bondVector, bondLength, &bondVector);

                FLOAT_TYPE expectedDistance = 0.5 * (particle.diameter + neighbor.diameter) * innerDiameterRatio;
                FLOAT_TYPE bondStrength = 0.5 * expectedDistance * lagrangeMultipliers[bondIndex];

                // Ensure error stabilization
                FLOAT_TYPE bondThreshold = bondsProvider->GetBondThreshold();
                FLOAT_TYPE gap = bondLength - expectedDistance;
                FLOAT_TYPE intersection = -gap;
                const FLOAT_TYPE maxFactor = 2.0;
                // Gap is too large
                if (gap > bondThreshold)
                {
                    FLOAT_TYPE maxGap = 5.0 * bondThreshold;
                    // If gap is maxGap, factor is maxFactor. If gap is bondThreshold, factor is 1.
                    FLOAT_TYPE factor = 1.0 + (maxFactor - 1.0) / (maxGap - bondThreshold) * (gap - bondThreshold);
                    // Decrease bond strength.
                    bondStrength /= factor;
                }
                else if (intersection > bondThreshold)
                {
                    FLOAT_TYPE maxIntersection = 5.0 * bondThreshold;
                    FLOAT_TYPE factor = 1.0 + (maxFactor - 1.0) / (maxIntersection - bondThreshold) * (intersection - bondThreshold);
                    // Increase bond strength.
                    bondStrength *= factor;
                }

                VectorUtilities::MultiplyByValue(bondVector, bondStrength, &bondVector);
                VectorUtilities::Add(velocity, bondVector, &velocity);
            }
        }
    }

    void ClosestJammingVelocityProvider::FillOptimizationMatrix(Eigen::SparseMatrix<FLOAT_TYPE>* optimizationMatrix)
    {
        typedef Eigen::Triplet<FLOAT_TYPE> MatrixEntry;
        vector<MatrixEntry> matrixEntries;
        int matrixEntriesCount = bondsProvider->GetBonds().size() + bondsProvider->GetBondPairsCount() * 2;
        matrixEntries.reserve(matrixEntriesCount);

        // Fill diagonal
        const Model::Packing& particlesRef = *particles;
        for (size_t i = 0; i < bondsProvider->GetBonds().size(); ++i)
        {
            const ParticlePair& bond = bondsProvider->GetBonds()[i];
            const DomainParticle& firstParticle = particlesRef[bond.firstParticleIndex];
            const DomainParticle& secondParticle = particlesRef[bond.secondParticleIndex];

            // This is a more stable computation with respect to bond breaking:
            // if due to finite precision or finite integration step a bond is broken by a small gap,
            // usage of the expected distance between particles, not the actual one (with the gap) may help close the gap
            // between particles in several steps (noticed experimentally).
            FLOAT_TYPE distance = 0.5 * (firstParticle.diameter + secondParticle.diameter) * innerDiameterRatio;
            FLOAT_TYPE distanceSquare = distance * distance;

            FLOAT_TYPE diagonalValue = 2.0 * distanceSquare;
            matrixEntries.push_back(MatrixEntry(i, i, diagonalValue));
        }

        // Non-diagonal values
        for (size_t particleIndex = 0; particleIndex < particles->size(); ++particleIndex)
        {
            const vector<BondsProvider::BondPair>& bondPairs = bondsProvider->GetBondPairsPerParticle()[particleIndex];
            for (size_t i = 0; i < bondPairs.size(); ++i)
            {
                const BondsProvider::BondPair& bondPair = bondPairs[i];

                const DomainParticle& commonParticle = particlesRef[bondPair.commonParticleIndex];
                const DomainParticle& firstNeighborParticle = particlesRef[bondPair.firstNeighborIndex];
                const DomainParticle& secondNeighborParticle = particlesRef[bondPair.secondNeighborIndex];

                SpatialVector vectorToFirstNeighbor;
                SpatialVector vectorToSecondNeighbor;

                // A more stable method with respect to accidental particle gaps removal
                mathService->FillDirection(firstNeighborParticle.coordinates, commonParticle.coordinates, &vectorToFirstNeighbor);
                mathService->FillDirection(secondNeighborParticle.coordinates, commonParticle.coordinates, &vectorToSecondNeighbor);
                FLOAT_TYPE commonToFirstDistance = 0.5 * (firstNeighborParticle.diameter + commonParticle.diameter) * innerDiameterRatio;
                FLOAT_TYPE commonToSecondDistance = 0.5 * (secondNeighborParticle.diameter + commonParticle.diameter) * innerDiameterRatio;
                FLOAT_TYPE nonDiagonalValue = commonToFirstDistance * commonToSecondDistance * VectorUtilities::GetDotProduct(vectorToFirstNeighbor, vectorToSecondNeighbor);

                matrixEntries.push_back(MatrixEntry(bondPair.firstBondIndex, bondPair.secondBondIndex, nonDiagonalValue));
                matrixEntries.push_back(MatrixEntry(bondPair.secondBondIndex, bondPair.firstBondIndex, nonDiagonalValue));
            }
        }

        optimizationMatrix->setFromTriplets(matrixEntries.begin(), matrixEntries.end());
    }

    void ClosestJammingVelocityProvider::FillOptimizationRightSide(Eigen::VectorXd* rightSide)
    {
        Eigen::VectorXd& rightSideRef = *rightSide;
        const Model::Packing& particlesRef = *particles;
        for (size_t i = 0; i < bondsProvider->GetBonds().size(); ++i)
        {
            const ParticlePair& bond = bondsProvider->GetBonds()[i];
            const DomainParticle& firstParticle = particlesRef[bond.firstParticleIndex];
            const DomainParticle& secondParticle = particlesRef[bond.secondParticleIndex];
            FLOAT_TYPE distance = 0.5 * (firstParticle.diameter + secondParticle.diameter);
            rightSideRef[i] = 2.0 * distance * distance * innerDiameterRatio;
        }
    }
}

