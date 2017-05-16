// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_PackingGenerator_h
#define Generation_PackingGenerators_Headers_PackingGenerator_h

#include <time.h>
#include "Core/Headers/Macros.h"
#include "IPackingGenerator.h"
namespace PackingGenerators { class IPackingStep; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { struct PackingSerializer; }
namespace PackingServices { class MathService; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class PackingGenerator : public virtual IPackingGenerator
    {
    private:
        // Services
        PackingServices::PackingSerializer* packingSerializer;
        PackingServices::GeometryService* geometryService;
        PackingServices::MathService* mathService;
        IPackingStep* packingStep;

        // Context
        const Model::ModellingContext* context;
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;
        const Model::GenerationConfig* generationConfig;

        // Working variables
        Core::FLOAT_TYPE innerDiameterRatio, outerDiameterRatio;

        Core::FLOAT_TYPE particlesVolume;
        Core::FLOAT_TYPE totalVolume;
        Core::FLOAT_TYPE theoreticalPorosity;

        Model::Packing* particles;

        static const Core::FLOAT_TYPE EPSILON;

    public:
        PackingGenerator(PackingServices::PackingSerializer* packingSerializer,
                PackingServices::GeometryService* geometryService,
                PackingServices::MathService* mathService,
                IPackingStep* packingStep);

        OVERRIDE void ArrangePacking(Model::Packing* particles);

        OVERRIDE Core::FLOAT_TYPE GetFinalInnerDiameterRatio() const;

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        virtual ~PackingGenerator();

    private:
        Core::FLOAT_TYPE CalculateTheoreticalPorosity() const;

        Core::FLOAT_TYPE CalculateCurrentPorosity(Core::FLOAT_TYPE diameterRatio) const;

        void Initialize();

        clock_t Log(unsigned long long iterationCounter) const;

        void Finish(clock_t totalTime, unsigned long long iterationCounter) const;

        // region Checking
        // All functions return shouldContinue

        void DisplayPorosity() const;

        void CheckIntersectionsNaive() const;

        // end region Checking

        DISALLOW_COPY_AND_ASSIGN(PackingGenerator);
    };
}

#endif /* Generation_PackingGenerators_Headers_PackingGenerator_h */

