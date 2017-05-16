// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_ClosestJammingVelocityProvider_h
#define Generation_PackingServices_Headers_ClosestJammingVelocityProvider_h

#include <ctime>
#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/ParticleCollisionService.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"

// Forward declaration doesn't work with Eigen types.
#include <Eigen/Sparse>

namespace PackingServices { class MathService; }
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class BondsProvider; }

namespace PackingServices
{
    class ClosestJammingVelocityProvider
    {
    private:
        // Services. TODO: Pass by pointer, when finally needed for testability.
        PackingGenerators::ParticleCollisionService particleCollisionService;
        MathService* mathService;
        const INeighborProvider* neighborProvider;
        const BondsProvider* bondsProvider;

        const Model::Packing* particles;
        std::vector<Core::SpatialVector>* particleVelocities;
        Core::FLOAT_TYPE innerDiameterRatio;

    public:
        ClosestJammingVelocityProvider(MathService* mathService);

        // This method does not follow a usual interface SetParticles/StartMove/EndMove,
        // 1. because neighborProvider, mathService and bondsProvider are shared with ClosestJammingStep,
        // so calls to their StartMove/EndMove will be duplicated.
        // 2. there may be a lot of moves in ClosestJammingStep between velocity calculations (so we don't have to follow each move).
        // neighborProvider and mathService are also passed here (not in the constructor) to emphasize that they should be synchronized with particles.
        clock_t FillVelocities(const INeighborProvider& neighborProvider,
                const BondsProvider& bondsProvider,
                const Model::Packing& particles,
                Core::FLOAT_TYPE innerDiameterRatio,
                std::vector<Core::SpatialVector>* particleVelocities);

        Core::FLOAT_TYPE FindBestMovementTime();

        ~ClosestJammingVelocityProvider();

    private:
        void FillOptimizationMatrix(Eigen::SparseMatrix<Core::FLOAT_TYPE>* optimizationMatrix);

        void FillOptimizationRightSide(Eigen::VectorXd* rightSide);

        void FillVelocities(const Eigen::VectorXd& lagrangeMultipliers);

        PackingGenerators::MovingParticle CreateMovingParticle(Model::ParticleIndex particleIndex);

//        void WriteLinearSystem(const Eigen::SparseMatrix<Core::FLOAT_TYPE>& optimizationMatrix, const Eigen::VectorXd& rightSide);

        DISALLOW_COPY_AND_ASSIGN(ClosestJammingVelocityProvider);
    };
}

#endif /* Generation_PackingServices_Headers_ClosestJammingVelocityProvider_h */
