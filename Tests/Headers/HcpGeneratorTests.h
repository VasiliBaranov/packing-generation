// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_HcpGeneratorTests_h
#define Headers_HcpGeneratorTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/Model/Headers/Types.h"
namespace PackingServices { class MathService; }
namespace PackingGenerators { class HcpGenerator; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace Tests
{
    class HcpGeneratorTests
    {
    private:
        static boost::shared_ptr<PackingServices::MathService> mathService;
        static boost::shared_ptr<PackingGenerators::HcpGenerator> hcpGenerator;

        static boost::shared_ptr<Geometries::IGeometry> geometry;
        static boost::shared_ptr<Model::SystemConfig> config;
        static boost::shared_ptr<Model::ModellingContext> context;
        static Model::Packing particles;

        static const Core::FLOAT_TYPE diameter;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void ArrangePacking_ForHcp_NoParticleIntersections();

        static void ArrangePacking_ForHcp_DensityCorrect();
    };
}

#endif /* Headers_HcpGeneratorTests_h */
