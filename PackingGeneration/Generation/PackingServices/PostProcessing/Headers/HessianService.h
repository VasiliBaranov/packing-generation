// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_HessianService_h
#define Generation_PackingServices_PostProcessing_Headers_HessianService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"
#include "Core/Headers/VectorUtilities.h"
namespace PackingServices { class RattlerRemovalService; }

namespace PackingServices
{
    // Represents a class to determine eigenvalues of a hessian.
    // See Xu, Blawzdziewicz, O�Hern (2005) Random close packing revisited: Ways to pack frictionless disks.
    class HessianService : public BaseDistanceService
    {
    public:
        struct SparseMatrixEntry
        {
            int rowIndex;
            int columnIndex;
            Core::FLOAT_TYPE value;

            SparseMatrixEntry() {};

            SparseMatrixEntry(int rowIndex, int columnIndex, Core::FLOAT_TYPE value)
            {
                this->rowIndex = rowIndex;
                this->columnIndex = columnIndex;
                this->value = value;
            }
        };

        struct SparseMatrix
        {
            std::vector<SparseMatrixEntry> values;
            int dimension;
        };

    private:
        static const Core::FLOAT_TYPE contractionRatio;
        RattlerRemovalService* rattlerRemovalService;

    public:
        HessianService(MathService* mathService, INeighborProvider* neighborProvider, RattlerRemovalService* rattlerRemovalService);

        void SetParticles(const Model::Packing& particles);

        void FillHessianEigenvalues(std::vector<Core::FLOAT_TYPE>* eigenvalues);

        // Methods to be tested in unit-tests
        void FillSymmetricMatrixEigenvaluesAsPacked(const SparseMatrix& matrix, std::vector<Core::FLOAT_TYPE>* eigenvalues);

        void FillSymmetricMatrixEigenvalues(const SparseMatrix& matrix, std::vector<Core::FLOAT_TYPE>* eigenvalues);

        void FillHessian(const std::vector<bool>& rattlerMask, Model::ParticleIndex nonRattlersCount, SparseMatrix* hessian);

        template<typename TPrecision>
        void ConvertSparseMatrixToNormal(const SparseMatrix& sparseMatrix, TPrecision** matrix)
        {
            Core::VectorUtilities::InitializeWith(matrix, 0.0, sparseMatrix.dimension, sparseMatrix.dimension);

            for (std::vector<SparseMatrixEntry>::const_iterator iterator = sparseMatrix.values.begin(); iterator != sparseMatrix.values.end(); ++iterator)
            {
                const SparseMatrixEntry& entry = *iterator;
                matrix[entry.rowIndex][entry.columnIndex] = entry.value;
            }
        }

    private:
        void FillSymmetricMatrixEigenvalues(float** matrix, int matrixDimension, float* eigenvalues);

        void FillSymmetricMatrixEigenvalues(float* matrixPacked, int matrixDimension, float* eigenvalues);

        void FillParticlePairHessian(SparseMatrix* hessian, const Model::DomainParticle& particle, const Model::DomainParticle& neighbor, Core::FLOAT_TYPE diagonalValues[DIMENSIONS][DIMENSIONS]);

        Core::FLOAT_TYPE GetHessianEntry(const Model::DomainParticle& particle, int firstParticleDimension, const Model::DomainParticle& neighbor, int secondParticleDimension, const Core::SpatialVector& direction, Core::FLOAT_TYPE distance);

        void FillPackedMatrix(const SparseMatrix& matrix, std::vector<float>* matrixPacked);

        // Index operations

        Model::ParticleIndex GetPackedMatrixSize(Model::ParticleIndex matrixDimension);

        Model::ParticleIndex GetHessianIndex(Model::ParticleIndex particleIndex, int dimension);

        Model::ParticleIndex GetPackedMatrixIndex(Model::ParticleIndex i, Model::ParticleIndex j);

        DISALLOW_COPY_AND_ASSIGN(HessianService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_HessianService_h */

