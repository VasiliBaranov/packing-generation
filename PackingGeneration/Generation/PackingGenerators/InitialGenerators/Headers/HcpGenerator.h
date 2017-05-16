// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_InitialGenerators_Headers_HcpGenerator_h
#define Generation_PackingGenerators_InitialGenerators_Headers_HcpGenerator_h

#include "Core/Headers/Macros.h"
#include "Generation/PackingGenerators/Headers/IPackingGenerator.h"
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class HcpGenerator : public virtual IPackingGenerator
    {
    private:
        // Context
        const Model::ModellingContext* context;
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;

        Model::Packing* particles;
        int currentParticleIndex;
        Core::FLOAT_TYPE radius;

        static const int expectedParticlesCount = (DIMENSIONS == 3) ? 48 : 12;

    public:
        HcpGenerator();

        OVERRIDE void ArrangePacking(Model::Packing* particles);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        OVERRIDE Core::FLOAT_TYPE GetFinalInnerDiameterRatio() const;

        static int GetExpectedParticlesCount();

        static void FillExpectedSize(Core::FLOAT_TYPE particleDiameter, Core::SpatialVector* expectedSize);

        virtual ~HcpGenerator();

    private:
        void Check() const;

        void AddRowAlongX(const Core::SpatialVector& firstParticleCenter);

        void AddLayerA(Core::SpatialVector* firstParticleCenter);

        void AddLayerB(Core::SpatialVector* firstParticleCenter);

        void AddLayer(Core::SpatialVector* firstParticleCenter, Core::FLOAT_TYPE rowShiftByX);

        DISALLOW_COPY_AND_ASSIGN(HcpGenerator);
    };
}

#endif /* Generation_PackingGenerators_InitialGenerators_Headers_HcpGenerator_h */

