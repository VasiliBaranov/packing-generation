// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_HessianServiceTests_h
#define Headers_HessianServiceTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/Model/Headers/Types.h"
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class HessianService; }
namespace PackingServices { class RattlerRemovalService; }
namespace PackingServices { class GeometryCollisionService; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace Tests
{
    class HessianServiceTests
    {
    private:
        static boost::shared_ptr<PackingServices::HessianService> hessianService;
        static boost::shared_ptr<PackingServices::MathService> mathService;
        static boost::shared_ptr<PackingServices::GeometryService> geometryService;
        static boost::shared_ptr<PackingServices::INeighborProvider> neighborProvider;
        static boost::shared_ptr<PackingServices::RattlerRemovalService> rattlerRemovalService;
        static boost::shared_ptr<PackingServices::GeometryCollisionService> geometryCollisionService;

        static boost::shared_ptr<Geometries::IGeometry> geometry;
        static boost::shared_ptr<Model::SystemConfig> config;
        static boost::shared_ptr<Model::ModellingContext> context;
        static Model::Packing particles;

        static int particlesCount;
        static Core::SpatialVector boxSize;
    public:
        static void RunTests();
    private:
        static void SetUp();

        static void TearDown();

        static void FillEigenvalues_ForTestMatrixAsPackedTwoDimensional_EigenvaluesCorrect();

        static void FillEigenvalues_ForTestMatrixTwoDimensional_EigenvaluesCorrect();

        static void FillEigenvalues_ForTestMatrixAsPackedThreeDimensional_EigenvaluesCorrect();

        static void FillEigenvalues_ForTestMatrixThreeDimensional_EigenvaluesCorrect();

        static void FillHessianEigenvalues_ForJammedPacking_NoExtraZeroEigenvalues();

        static void FillHessian_ForJammedPacking_HessianSymmetric();

        static void FillHessian_ForJammedPacking_RowsHaveZeroSum();

        static void FillSampleSystem();
    };
}

#endif /* Headers_HessianServiceTests_h */
