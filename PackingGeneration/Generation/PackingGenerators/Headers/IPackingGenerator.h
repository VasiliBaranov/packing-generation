// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_IPackingGenerator_h
#define Generation_PackingGenerators_Headers_IPackingGenerator_h

#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace Model { class GenerationConfig; }

namespace PackingGenerators
{
    // Defines methods for packing generation.
    class IPackingGenerator : public virtual PackingServices::IContextDependentService
    {
    public:
        virtual void SetGenerationConfig(const Model::GenerationConfig& generationConfig) = 0;

        virtual void ArrangePacking(Model::Packing* particles) = 0;

        virtual Core::FLOAT_TYPE GetFinalInnerDiameterRatio() const = 0;

        virtual ~IPackingGenerator(){ };
    };
}


#endif /* Generation_PackingGenerators_Headers_IPackingGenerator_h */

