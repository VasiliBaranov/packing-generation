// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_Headers_IPackingStep_h
#define Generation_PackingGenerators_Headers_IPackingStep_h

#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace Model { class GenerationConfig; }

namespace PackingGenerators
{
    class IPackingStep : public virtual PackingServices::IContextDependentService
    {
    public:
        virtual void SetGenerationConfig(const Model::GenerationConfig& generationConfig) = 0;

        virtual void SetParticles(Model::Packing* particles) = 0;

        virtual void DisplaceParticles() = 0;

        virtual void ResetGeneration() = 0;

        virtual bool ShouldContinue() const = 0;

        virtual Core::FLOAT_TYPE GetInnerDiameterRatio() const = 0;

        virtual Core::FLOAT_TYPE GetOuterDiameterRatio() const = 0;

        virtual ~IPackingStep(){ };
    };
}

#endif /* Generation_PackingGenerators_Headers_IPackingStep_h */

