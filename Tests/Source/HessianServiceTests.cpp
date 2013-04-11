// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/HessianServiceTests.h"

#include "../Headers/Assert.h"
#include "Core/Headers/MemoryUtility.h"
#include "Core/Headers/StlUtilities.h"
#include "Generation/Geometries/Headers/BulkGeometry.h"
#include "Generation/Model/Headers/Config.h"

#include "Generation/PackingServices/DistanceServices/Headers/CellListNeighborProvider.h"

#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"

#include "Generation/PackingServices/PostProcessing/Headers/HessianService.h"
#include "Generation/PackingServices/PostProcessing/Headers/RattlerRemovalService.h"

#include "Generation/PackingGenerators/InitialGenerators/Headers/HcpGenerator.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Geometries;
using namespace PackingServices;
using namespace PackingGenerators;

namespace Tests
{
    boost::shared_ptr<HessianService> HessianServiceTests::hessianService;
    boost::shared_ptr<MathService> HessianServiceTests::mathService;
    boost::shared_ptr<GeometryService> HessianServiceTests::geometryService;
    boost::shared_ptr<INeighborProvider> HessianServiceTests::neighborProvider;
    boost::shared_ptr<RattlerRemovalService> HessianServiceTests::rattlerRemovalService;
    boost::shared_ptr<GeometryCollisionService> HessianServiceTests::geometryCollisionService;

    boost::shared_ptr<IGeometry> HessianServiceTests::geometry;
    boost::shared_ptr<SystemConfig> HessianServiceTests::config;
    boost::shared_ptr<ModellingContext> HessianServiceTests::context;

    Packing HessianServiceTests::particles;
    int HessianServiceTests::particlesCount = 4;
    SpatialVector HessianServiceTests::boxSize = {{10.0, 10.0, 10.0}};

    void HessianServiceTests::SetUp()
    {
        mathService.reset(new MathService());
        geometryService.reset(new GeometryService(mathService.get()));
        geometryCollisionService.reset(new GeometryCollisionService());
        neighborProvider.reset(new CellListNeighborProvider(geometryService.get(), geometryCollisionService.get()));
        rattlerRemovalService.reset(new RattlerRemovalService(mathService.get(), neighborProvider.get()));
        hessianService.reset(new HessianService(mathService.get(), neighborProvider.get(), rattlerRemovalService.get()));

        config.reset(new SystemConfig());
        config->packingSize = boxSize;
        config->particlesCount = particlesCount;
        config->boundariesMode = BoundariesMode::Bulk;

        geometry.reset(new BulkGeometry(*config));

        context.reset(new ModellingContext(config.get(), geometry.get()));

        mathService->SetContext(*context);
        geometryService->SetContext(*context);
        neighborProvider->SetContext(*context);
        hessianService->SetContext(*context);
        rattlerRemovalService->SetContext(*context);
        rattlerRemovalService->SetMinNeighborsCount(1);

        particles.resize(particlesCount);
    }

    void HessianServiceTests::TearDown()
    {
    }

    void HessianServiceTests::FillEigenvalues_ForTestMatrixAsPackedTwoDimensional_EigenvaluesCorrect()
    {
        SetUp();

//        a =
//        1     2
//        2     4

        const int dimensions = 2;
        HessianService::SparseMatrix matrix;
        matrix.dimension = dimensions;
        matrix.values.reserve(dimensions * dimensions);

        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 0, 1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 1, 2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 0, 2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 1, 4));

        vector<FLOAT_TYPE> eigenvalues(dimensions);
        hessianService->FillSymmetricMatrixEigenvaluesAsPacked(matrix, &eigenvalues);

        boost::array<FLOAT_TYPE, 2> expectedEigenvalues = {{0.0, 5.0}};
        Assert::AreVectorsAlmostEqual(eigenvalues, expectedEigenvalues, "FillEigenvalues_ForTestMatrixAsPackedTwoDimensional_EigenvaluesCorrect");

        TearDown();
    }

    void HessianServiceTests::FillEigenvalues_ForTestMatrixTwoDimensional_EigenvaluesCorrect()
    {
        SetUp();

//        a =
//        1     2
//        2     4

        const int dimensions = 2;
        HessianService::SparseMatrix matrix;
        matrix.dimension = dimensions;
        matrix.values.reserve(dimensions * dimensions);

        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 0, 1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 1, 2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 0, 2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 1, 4));

        vector<FLOAT_TYPE> eigenvalues(dimensions);
        hessianService->FillSymmetricMatrixEigenvalues(matrix, &eigenvalues);

        boost::array<FLOAT_TYPE, 2> expectedEigenvalues = {{0.0, 5.0}};
        Assert::AreVectorsAlmostEqual(eigenvalues, expectedEigenvalues, "FillEigenvalues_ForTestMatrixTwoDimensional_EigenvaluesCorrect");

        TearDown();
    }

    void HessianServiceTests::FillEigenvalues_ForTestMatrixAsPackedThreeDimensional_EigenvaluesCorrect()
    {
        SetUp();

//        a =
//        3    -1    -2
//       -1     4    -3
//       -2    -3     5

        const int dimensions = 3;
        HessianService::SparseMatrix matrix;
        matrix.dimension = dimensions;
        matrix.values.reserve(dimensions * dimensions);

        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 0, 3));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 1, -1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 2, -2));

        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 0, -1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 1, 4));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 2, -3));

        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 0, -2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 1, -3));
        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 2, 5));

        vector<FLOAT_TYPE> eigenvalues(dimensions);
        hessianService->FillSymmetricMatrixEigenvaluesAsPacked(matrix, &eigenvalues);

        boost::array<FLOAT_TYPE, 3> expectedEigenvalues = {{ -6.66133814775094e-16, 4.26794919243112, 7.73205080756888 }};
        Assert::AreVectorsAlmostEqual(eigenvalues, expectedEigenvalues, "FillEigenvalues_ForTestMatrixAsPackedThreeDimensional_EigenvaluesCorrect");

        TearDown();
    }

    void HessianServiceTests::FillEigenvalues_ForTestMatrixThreeDimensional_EigenvaluesCorrect()
    {
        SetUp();

//        a =
//        3    -1    -2
//       -1     4    -3
//       -2    -3     5

        const int dimensions = 3;
        HessianService::SparseMatrix matrix;
        matrix.dimension = dimensions;
        matrix.values.reserve(dimensions * dimensions);

        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 0, 3));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 1, -1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(0, 2, -2));

        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 0, -1));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 1, 4));
        matrix.values.push_back(HessianService::SparseMatrixEntry(1, 2, -3));

        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 0, -2));
        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 1, -3));
        matrix.values.push_back(HessianService::SparseMatrixEntry(2, 2, 5));

        vector<FLOAT_TYPE> eigenvalues(dimensions);
        hessianService->FillSymmetricMatrixEigenvalues(matrix, &eigenvalues);

        boost::array<FLOAT_TYPE, 3> expectedEigenvalues = {{ -6.66133814775094e-16, 4.26794919243112, 7.73205080756888 }};
        Assert::AreVectorsAlmostEqual(eigenvalues, expectedEigenvalues, "FillEigenvalues_ForTestMatrixThreeDimensional_EigenvaluesCorrect");

        TearDown();
    }

    void HessianServiceTests::FillHessianEigenvalues_ForJammedPacking_NoExtraZeroEigenvalues()
    {
        // Arrange
        FillSampleSystem();

        hessianService->SetParticles(particles);
        vector<FLOAT_TYPE> hessianEigenvalues;
        hessianEigenvalues.reserve(particlesCount * DIMENSIONS);

        // Act
        hessianService->FillHessianEigenvalues(&hessianEigenvalues);

        // Assert
        int negativeEigenvaluesCount = 0;
        int zeroEigenvaluesCount = 0;

        const FLOAT_TYPE epsilon = 1e-6;
        for (vector<FLOAT_TYPE>::iterator iterator = hessianEigenvalues.begin(); iterator != hessianEigenvalues.end(); iterator++)
        {
            FLOAT_TYPE eigenvalue = *iterator;
            if (eigenvalue < - epsilon)
            {
                negativeEigenvaluesCount++;
            }

            if (std::abs(eigenvalue) <= epsilon)
            {
                zeroEigenvaluesCount++;
            }
        }
        Assert::AreEqual(negativeEigenvaluesCount, 0, "FillHessianEigenvalues_ForJammedPacking_NoExtraZeroEigenvalues");
        Assert::AreEqual(zeroEigenvaluesCount, DIMENSIONS, "FillHessianEigenvalues_ForJammedPacking_NoExtraZeroEigenvalues");

        TearDown();
    }

    void HessianServiceTests::FillHessian_ForJammedPacking_HessianSymmetric()
    {
        // Arrange
        FillSampleSystem();
        hessianService->SetParticles(particles);

        vector<bool> rattlerMask(particlesCount);
        rattlerRemovalService->FillRattlerMask(0.999, &rattlerMask);
        int nonRattlersCount = rattlerRemovalService->FindNonRattlersCount(rattlerMask);

        // Act
        HessianService::SparseMatrix hessianSparse;
        hessianService->FillHessian(rattlerMask, nonRattlersCount, &hessianSparse);

        FLOAT_TYPE** hessian = MemoryUtility::Allocate2DArray<FLOAT_TYPE>(hessianSparse.dimension, hessianSparse.dimension);
        hessianService->ConvertSparseMatrixToNormal(hessianSparse, hessian);

//        EndiannessProvider endiannessProvider;
//        PackingSerializer serializer(&endiannessProvider);
//        serializer.SerializeMatrix("/cygdrive/e/temp/HessianSymmetric.txt", hessian, hessianSparse.dimension);

        // Assert
        bool hessianSymmetric = true;
        for (int i = 0; i < hessianSparse.dimension; i++)
        {
            for (int j = 0; j < hessianSparse.dimension; j++)
            {
                if (std::abs(hessian[i][j] - hessian[j][i]) > 1e-10)
                {
                    hessianSymmetric = false;
                }
            }
        }

        MemoryUtility::Free2DArray(hessian);
        Assert::IsTrue(hessianSymmetric, "FillHessian_ForJammedPacking_HessianSymmetric");

        TearDown();
    }

    void HessianServiceTests::FillHessian_ForJammedPacking_RowsHaveZeroSum()
    {
        // Arrange
        FillSampleSystem();
        hessianService->SetParticles(particles);

        vector<bool> rattlerMask(particlesCount);
        rattlerRemovalService->FillRattlerMask(0.999, &rattlerMask);
        int nonRattlersCount = rattlerRemovalService->FindNonRattlersCount(rattlerMask);

        // Act
        HessianService::SparseMatrix hessianSparse;
        hessianService->FillHessian(rattlerMask, nonRattlersCount, &hessianSparse);

        FLOAT_TYPE** hessian = MemoryUtility::Allocate2DArray<FLOAT_TYPE>(hessianSparse.dimension, hessianSparse.dimension);
        hessianService->ConvertSparseMatrixToNormal(hessianSparse, hessian);

        // Assert
        bool rowsHaveZeroSum = true;
        for (int i = 0; i < hessianSparse.dimension; i++)
        {
            vector<FLOAT_TYPE> row(hessian[i], hessian[i] + hessianSparse.dimension);
            FLOAT_TYPE sum = VectorUtilities::Sum(row);
            if (std::abs(sum) > 1e-10)
            {
                rowsHaveZeroSum = false;
                break;
            }
        }

        MemoryUtility::Free2DArray(hessian);
        Assert::IsTrue(rowsHaveZeroSum, "FillHessian_ForJammedPacking_RowsHaveZeroSum");

        TearDown();
    }

    // Arrange particles in HCP lattice.
    // I also tried arranging in a periodic cubic lattice, but the latter does produce negative and extra zero eigenvalues, as is not optimal.
    void HessianServiceTests::FillSampleSystem()
    {
        const FLOAT_TYPE r = 0.5;
        particlesCount = HcpGenerator::GetExpectedParticlesCount();
        HcpGenerator::FillExpectedSize(2.0 * r, &boxSize);

        SetUp();

        for (int i = 0; i < particlesCount; i++)
        {
            particles[i].index = i;
            particles[i].diameter = 2.0 * r;
        }

        HcpGenerator hcpGenerator;
        hcpGenerator.SetContext(*context);
//        hcpGenerator.SetGenerationConfig(NULL);

        hcpGenerator.ArrangePacking(&particles);
    }

    void HessianServiceTests::RunTests()
    {
        FillEigenvalues_ForTestMatrixAsPackedTwoDimensional_EigenvaluesCorrect();
        FillEigenvalues_ForTestMatrixTwoDimensional_EigenvaluesCorrect();
        FillEigenvalues_ForTestMatrixAsPackedThreeDimensional_EigenvaluesCorrect();
        FillEigenvalues_ForTestMatrixThreeDimensional_EigenvaluesCorrect();
        FillHessianEigenvalues_ForJammedPacking_NoExtraZeroEigenvalues();
        FillHessian_ForJammedPacking_HessianSymmetric();
        FillHessian_ForJammedPacking_RowsHaveZeroSum();
    }
}


