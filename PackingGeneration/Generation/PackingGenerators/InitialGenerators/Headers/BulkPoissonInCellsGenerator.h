// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_InitialGenerators_Headers_BulkPoissonInCellsGenerator_h
#define Generation_PackingGenerators_InitialGenerators_Headers_BulkPoissonInCellsGenerator_h

#include "Core/Headers/Macros.h"
#include "Generation/PackingGenerators/Headers/IPackingGenerator.h"
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }

namespace PackingGenerators
{
    class BulkPoissonInCellsGenerator : public virtual IPackingGenerator
    {
    private:
        // Context
        const Model::ModellingContext* context;
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;
        static const int PARTICLES_IN_CELL = 10;

    public:
        BulkPoissonInCellsGenerator();

        OVERRIDE void ArrangePacking(Model::Packing* particles);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetGenerationConfig(const Model::GenerationConfig& generationConfig);

        OVERRIDE Core::FLOAT_TYPE GetFinalInnerDiameterRatio() const;

        virtual ~BulkPoissonInCellsGenerator();

    private:
        DISALLOW_COPY_AND_ASSIGN(BulkPoissonInCellsGenerator);
    };
}

#endif /* Generation_PackingGenerators_InitialGenerators_Headers_BulkPoissonInCellsGenerator_h */

