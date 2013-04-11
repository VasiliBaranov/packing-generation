// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/HessianService.h"

#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/MemoryUtility.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "../Headers/RattlerRemovalService.h"

#ifdef LAPACK_AVAILABLE
#include "lapacke.h"
#endif

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    const FLOAT_TYPE HessianService::contractionRatio = 0.999;

    HessianService::HessianService(MathService* mathService, INeighborProvider* neighborProvider, RattlerRemovalService* rattlerRemovalService) :
            BaseDistanceService(mathService, neighborProvider)
    {
        this->rattlerRemovalService = rattlerRemovalService;
    }

    void HessianService::SetParticles(const Packing& particles)
    {
        this->particles = &particles;
        neighborProvider->SetParticles(particles);
    }

    void HessianService::FillHessianEigenvalues(vector<FLOAT_TYPE>* eigenvalues)
    {
        vector<bool> rattlerMask(config->particlesCount);
        rattlerRemovalService->FillRattlerMask(contractionRatio, &rattlerMask);
        ParticleIndex nonRattlersCount = rattlerRemovalService->FindNonRattlersCount(rattlerMask);

        SparseMatrix hessian;
        FillHessian(rattlerMask, nonRattlersCount, &hessian);

//        FillSymmetricMatrixEigenvaluesAsPacked(hessian, eigenvalues); Passes all tests, but doesn't work for real systems. Don't use it.
        FillSymmetricMatrixEigenvalues(hessian, eigenvalues);
    }

    void HessianService::FillHessian(const vector<bool>& rattlerMask, ParticleIndex nonRattlersCount, SparseMatrix* hessian)
    {
        const Packing& particlesRef = *particles;
        // N particles, 6 contacts per particle, 3 entries per contact (one per spatial dimension). Therefore, 18N entries. While the entire matrix will be 3N by 3N.
        hessian->values.reserve(18 * nonRattlersCount);
        hessian->dimension = nonRattlersCount * DIMENSIONS;

        FLOAT_TYPE diagonalValues[DIMENSIONS][DIMENSIONS];

        ParticleIndex correctParticleIndex = 0;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            if (rattlerMask[particleIndex])
            {
                continue;
            }

            const DomainParticle& particle = particlesRef[particleIndex];
            VectorUtilities::InitializeWith(diagonalValues, 0.0, DIMENSIONS, DIMENSIONS);

            ParticleIndex neighborsCount;
            const ParticleIndex* neighborIndexes = neighborProvider->GetNeighborIndexes(particleIndex, &neighborsCount);
            for (ParticleIndex i = 0; i < neighborsCount; ++i)
            {
                ParticleIndex neighborIndex = neighborIndexes[i];
                if (rattlerMask[neighborIndex])
                {
                    continue;
                }

                const DomainParticle& neighbor = particlesRef[neighborIndex];
                FillParticlePairHessian(hessian, particle, neighbor, diagonalValues);
            }

            for (int firstParticleDimension = 0; firstParticleDimension < DIMENSIONS; ++firstParticleDimension)
            {
                for (int secondParticleDimension = 0; secondParticleDimension < DIMENSIONS; ++secondParticleDimension)
                {
                    int rowIndex = GetHessianIndex(correctParticleIndex, firstParticleDimension);
                    int columnIndex = GetHessianIndex(correctParticleIndex, secondParticleDimension);
                    FLOAT_TYPE hessianEntry = -diagonalValues[firstParticleDimension][secondParticleDimension];
                    SparseMatrixEntry entry(rowIndex, columnIndex, hessianEntry);

                    hessian->values.push_back(entry);
                }
            }

            correctParticleIndex++;
        }
    }

    void HessianService::FillParticlePairHessian(SparseMatrix* hessian, const DomainParticle& particle, const DomainParticle& neighbor, FLOAT_TYPE diagonalValues[DIMENSIONS][DIMENSIONS])
    {
        SpatialVector difference;
        SpatialVector direction;

        FLOAT_TYPE normalizedDistance = mathService->GetNormalizedDistance(particle, neighbor);

        FLOAT_TYPE contractedDiameterRatio = normalizedDistance * contractionRatio;
        if (contractedDiameterRatio > 1.0)
        {
            return;
        }

        mathService->FillDistance(neighbor.coordinates, particle.coordinates, &difference);
        FLOAT_TYPE distance = normalizedDistance * (particle.diameter + neighbor.diameter) * 0.5; // or VectorUtilities::GetLength(difference);
        VectorUtilities::DivideByValue(difference, distance, &direction);

        for (int firstParticleDimension = 0; firstParticleDimension < DIMENSIONS; ++firstParticleDimension)
        {
            for (int secondParticleDimension = 0; secondParticleDimension < DIMENSIONS; ++secondParticleDimension)
            {
                FLOAT_TYPE hessianEntry = GetHessianEntry(particle, firstParticleDimension, neighbor, secondParticleDimension, direction, distance);
                int rowIndex = GetHessianIndex(particle.index, firstParticleDimension);
                int columnIndex = GetHessianIndex(neighbor.index, secondParticleDimension);
                SparseMatrixEntry entry(rowIndex, columnIndex, hessianEntry);

                hessian->values.push_back(entry);
                diagonalValues[firstParticleDimension][secondParticleDimension] += hessianEntry;
            }
        }
    }

    // See Xu, Blawzdziewicz, O�Hern (2005) Random close packing revisited: Ways to pack frictionless disks.
    FLOAT_TYPE HessianService::GetHessianEntry(const DomainParticle& particle, int firstParticleDimension, const DomainParticle& neighbor, int secondParticleDimension, const SpatialVector& direction, FLOAT_TYPE distance)
    {
        const FLOAT_TYPE potentialNormalizer = 1.0;

        FLOAT_TYPE halfDiameter = (particle.diameter + neighbor.diameter) * 0.5;
        distance *= contractionRatio; // as if particles are contracted and overlapping, giving rise to the potential energy

        FLOAT_TYPE potentialFirstDerivative = - potentialNormalizer * (1.0 - distance / halfDiameter) / halfDiameter;
        FLOAT_TYPE potentialSecondDerivative = potentialNormalizer / halfDiameter / halfDiameter;

        FLOAT_TYPE delta = VectorUtilities::GetKroneckerDelta(firstParticleDimension, secondParticleDimension);
        FLOAT_TYPE firstDerivativeEntry = - potentialFirstDerivative / distance * (delta - direction[firstParticleDimension] * direction[secondParticleDimension]);
        FLOAT_TYPE secondDerivativeEntry = - potentialSecondDerivative * direction[firstParticleDimension] * direction[secondParticleDimension];
        FLOAT_TYPE hessianEntry = firstDerivativeEntry + secondDerivativeEntry;

        return hessianEntry;

        // Hessian entry means increase in force on the second particle in the secondParticleDimension, when shifting the first particle in the firstParticleDimension, multiplied by -1.
        // Negative hessian entry means the increase of force, positive hessian entry means the decrease of force.
        // We consider hard sphere packings, so the force is initially zero, and it can not decrease, can just increase with intersections.
        // if (hessianEntry > 0)
        // {
        //     continue;
        // }
    }

    ParticleIndex HessianService::GetPackedMatrixSize(ParticleIndex matrixDimension)
    {
        return (matrixDimension * (matrixDimension + 1)) / 2;
    }

    // See http://www.netlib.org/lapack/lug/node123.html
    ParticleIndex HessianService::GetPackedMatrixIndex(ParticleIndex i, ParticleIndex j)
    {
        bool lowerHalf = i > j;
        if (lowerHalf)
        {
            // Swap indexes to go upper half (valid, as the matrix is symmetric)
            ParticleIndex temp = j;
            j = i;
            i = temp;
        }

        // Convert to one-based indexes
        i++;
        j++;

        // Convert to zero-based indexes
        return i + (j * (j - 1)) / 2 - 1;
    }

    ParticleIndex HessianService::GetHessianIndex(ParticleIndex particleIndex, int dimension)
    {
        return particleIndex * DIMENSIONS + dimension;
    }

    void HessianService::FillSymmetricMatrixEigenvaluesAsPacked(const SparseMatrix& matrix, vector<FLOAT_TYPE>* eigenvalues)
    {
        vector<float> matrixPacked;
        FillPackedMatrix(matrix, &matrixPacked);

        // Use float, not FLOAT_TYPE, as matrices are huge.
        vector<float> lapackEigenvalues(matrix.dimension);
        FillSymmetricMatrixEigenvalues(&matrixPacked[0], matrix.dimension, &lapackEigenvalues[0]);
        StlUtilities::Append(lapackEigenvalues, eigenvalues);
    }

    void HessianService::FillSymmetricMatrixEigenvalues(const SparseMatrix& matrixSparse, vector<FLOAT_TYPE>* eigenvalues)
    {
        float** matrix = MemoryUtility::Allocate2DArray<float>(matrixSparse.dimension, matrixSparse.dimension);
        ConvertSparseMatrixToNormal(matrixSparse, matrix);

        // Use float, not FLOAT_TYPE, as matrices are huge.
        vector<float> lapackEigenvalues(matrixSparse.dimension);
        FillSymmetricMatrixEigenvalues(matrix, matrixSparse.dimension, &lapackEigenvalues[0]);
        StlUtilities::Append(lapackEigenvalues, eigenvalues);

        MemoryUtility::Free2DArray(matrix);
    }

    void HessianService::FillPackedMatrix(const SparseMatrix& matrix, vector<float>* matrixPacked)
    {
        vector<SparseMatrixEntry>::const_iterator iterator;
        vector<float>& matrixPackedRef = *matrixPacked;

        int packedMatrixSize = GetPackedMatrixSize(matrix.dimension);
        matrixPackedRef.resize(packedMatrixSize);

        for (iterator = matrix.values.begin(); iterator != matrix.values.end(); ++iterator)
        {
            const SparseMatrixEntry& entry = *iterator;
            ParticleIndex i = GetPackedMatrixIndex(entry.rowIndex, entry.columnIndex);
            matrixPackedRef[i] = entry.value;
        }
    }

    void HessianService::FillSymmetricMatrixEigenvalues(float* matrixPacked, int matrixDimension, float* eigenvalues)
    {
//         Fortran declaration

//        SSPEV computes all the eigenvalues and, optionally, eigenvectors of a
//        real symmetric matrix A in packed storage.

//        subroutine SSPEV    (   CHARACTER   JOBZ,
//                                CHARACTER   UPLO,
//                                INTEGER     N,
//                                REAL, dimension( * )    AP,
//                                REAL, dimension( * )    W,
//                                REAL, dimension( ldz, * )   Z,
//                                INTEGER     LDZ,
//                                REAL, dimension( * )    WORK,
//                                INTEGER     INFO
//                                )

//        [in]    JOBZ
//                  JOBZ is CHARACTER*1
//                  = 'N':  Compute eigenvalues only;
//                  = 'V':  Compute eigenvalues and eigenvectors.
//
//        [in]    UPLO
//                  UPLO is CHARACTER*1
//                  = 'U':  Upper triangle of A is stored;
//                  = 'L':  Lower triangle of A is stored.
//
//        [in]    N
//                  N is INTEGER
//                  The order of the matrix A.  N >= 0.
//
//        [in,out]    AP
//                  AP is REAL array, dimension (N*(N+1)/2)
//                  On entry, the upper or lower triangle of the symmetric matrix
//                  A, packed columnwise in a linear array.  The j-th column of A
//                  is stored in the array AP as follows:
//                  if UPLO = 'U', AP(i + (j-1)*j/2) = A(i,j) for 1<=i<=j;
//                  if UPLO = 'L', AP(i + (j-1)*(2*n-j)/2) = A(i,j) for j<=i<=n.
//
//                  On exit, AP is overwritten by values generated during the
//                  reduction to tridiagonal form.  If UPLO = 'U', the diagonal
//                  and first superdiagonal of the tridiagonal matrix T overwrite
//                  the corresponding elements of A, and if UPLO = 'L', the
//                  diagonal and first subdiagonal of T overwrite the
//                  corresponding elements of A.
//
//        [out]   W
//                  W is REAL array, dimension (N)
//                  If INFO = 0, the eigenvalues in ascending order.
//
//        [out]   Z
//                  Z is REAL array, dimension (LDZ, N)
//                  If JOBZ = 'V', then if INFO = 0, Z contains the orthonormal
//                  eigenvectors of the matrix A, with the i-th column of Z
//                  holding the eigenvector associated with W(i).
//                  If JOBZ = 'N', then Z is not referenced.
//
//        [in]    LDZ
//                  LDZ is INTEGER
//                  The leading dimension of the array Z.  LDZ >= 1, and if
//                  JOBZ = 'V', LDZ >= max(1,N).
//
//        [out]   WORK
//                  WORK is REAL array, dimension (3*N)
//
//        [out]   INFO
//                  INFO is INTEGER
//                  = 0:  successful exit.
//                  < 0:  if INFO = -i, the i-th argument had an illegal value.
//                  > 0:  if INFO = i, the algorithm failed to converge; i
//                        off-diagonal elements of an intermediate tridiagonal
//                        form did not converge to zero.

//         C++ declaration
//        lapack_int LAPACKE_sspev( int matrix_order, char jobz, char uplo, lapack_int n,
//                                  float* ap, float* w, float* z, lapack_int ldz );
#ifdef LAPACK_AVAILABLE
        lapack_int info;

        // Specify LAPACK_COL_MAJOR to comply with the initial fortran specification (i.e. that UPLO == 'U' corresponds to the upper triangle of the matrix in AP).
        // When specifying LAPACK_ROW_MAJOR and 'U' AP should contain lower triangle matrix values.
        info = LAPACKE_sspev(LAPACK_COL_MAJOR, 'N', 'U', matrixDimension, matrixPacked, eigenvalues, NULL, matrixDimension);

        if (info != 0)
        {
            throw InvalidOperationException("Error in computing hessian eigenvalues.");
        }
#else

#endif
    }

    void HessianService::FillSymmetricMatrixEigenvalues(float** matrix, int matrixDimension, float* eigenvalues)
    {
        //        subroutine SSYEV    (   CHARACTER   JOBZ,
        //                                CHARACTER   UPLO,
        //                                INTEGER     N,
        //                                REAL, dimension( lda, * )   A,
        //                                INTEGER     LDA,
        //                                REAL, dimension( * )    W,
        //                                REAL, dimension( * )    WORK,
        //                                INTEGER     LWORK,
        //                                INTEGER     INFO
        //                                )

        //        [in]    JOBZ
        //                  JOBZ is CHARACTER*1
        //                  = 'N':  Compute eigenvalues only;
        //                  = 'V':  Compute eigenvalues and eigenvectors.
        //
        //        [in]    UPLO
        //                  UPLO is CHARACTER*1
        //                  = 'U':  Upper triangle of A is stored;
        //                  = 'L':  Lower triangle of A is stored.
        //
        //        [in]    N
        //                  N is INTEGER
        //                  The order of the matrix A.  N >= 0.
        //
        //        [in,out]    A
        //                  A is REAL array, dimension (LDA, N)
        //                  On entry, the symmetric matrix A.  If UPLO = 'U', the
        //                  leading N-by-N upper triangular part of A contains the
        //                  upper triangular part of the matrix A.  If UPLO = 'L',
        //                  the leading N-by-N lower triangular part of A contains
        //                  the lower triangular part of the matrix A.
        //                  On exit, if JOBZ = 'V', then if INFO = 0, A contains the
        //                  orthonormal eigenvectors of the matrix A.
        //                  If JOBZ = 'N', then on exit the lower triangle (if UPLO='L')
        //                  or the upper triangle (if UPLO='U') of A, including the
        //                  diagonal, is destroyed.
        //
        //        [in]    LDA
        //                  LDA is INTEGER
        //                  The leading dimension of the array A.  LDA >= max(1,N).
        //
        //        [out]   W
        //                  W is REAL array, dimension (N)
        //                  If INFO = 0, the eigenvalues in ascending order.
        //
        //        [out]   WORK
        //                  WORK is REAL array, dimension (MAX(1,LWORK))
        //                  On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
        //
        //        [in]    LWORK
        //                  LWORK is INTEGER
        //                  The length of the array WORK.  LWORK >= max(1,3*N-1).
        //                  For optimal efficiency, LWORK >= (NB+2)*N,
        //                  where NB is the blocksize for SSYTRD returned by ILAENV.
        //
        //                  If LWORK = -1, then a workspace query is assumed; the routine
        //                  only calculates the optimal size of the WORK array, returns
        //                  this value as the first entry of the WORK array, and no error
        //                  message related to LWORK is issued by XERBLA.
        //
        //        [out]   INFO
        //                  INFO is INTEGER
        //                  = 0:  successful exit
        //                  < 0:  if INFO = -i, the i-th argument had an illegal value
        //                  > 0:  if INFO = i, the algorithm failed to converge; i
        //                        off-diagonal elements of an intermediate tridiagonal
        //                        form did not converge to zero.

        //        lapack_int LAPACKE_ssyev( int matrix_order, char jobz, char uplo, lapack_int n,
        //                                  float* a, lapack_int lda, float* w );
#ifdef LAPACK_AVAILABLE
        lapack_int info;

        // LAPACK_COLUMN_MAJOR corresponds to the initial FORTRAN matrix storage convention
        // LAPACK_ROW_MAJOR corresponds to the C matrix storage convention
        info = LAPACKE_ssyev(LAPACK_ROW_MAJOR, 'N', 'U', matrixDimension, *matrix, matrixDimension, eigenvalues);

        if (info != 0)
        {
            throw InvalidOperationException("Error in computing hessian eigenvalues.");
        }
#else

#endif
    }
}

