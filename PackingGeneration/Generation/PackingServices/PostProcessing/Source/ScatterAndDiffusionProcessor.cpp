// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ScatterAndDiffusionProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/SelfDiffusionProcessor.h"
#include "Generation/PackingServices/PostProcessing/Headers/IntermediateScatteringFunctionProcessor.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingGenerators;

namespace PackingServices
{
    ScatterAndDiffusionProcessor::ScatterAndDiffusionProcessor(IntermediateScatteringFunctionProcessor* intermediateScatteringFunctionProcessor, SelfDiffusionProcessor* selfDiffusionProcessor)
    {
        this->selfDiffusionProcessor = selfDiffusionProcessor;
        this->intermediateScatteringFunctionProcessor = intermediateScatteringFunctionProcessor;

        shouldResetSelfDiffusionProcessor = this->selfDiffusionProcessor->ResetsExternally();
    }

    void ScatterAndDiffusionProcessor::Start()
    {
        intermediateScatteringFunctionProcessor->Start();
        selfDiffusionProcessor->Start();
    }

    EquilibrationProcessingStatus::Type ScatterAndDiffusionProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        EquilibrationProcessingStatus::Type scatterResult = intermediateScatteringFunctionProcessor->ProcessStep(particles, statistics);

        if (shouldResetSelfDiffusionProcessor)
        {
            if (intermediateScatteringFunctionProcessor->ReferencePackingAddedOnCurrentStep())
            {
                selfDiffusionProcessor->ResetReferencePacking();
            }
        }

        EquilibrationProcessingStatus::Type diffusionResult = selfDiffusionProcessor->ProcessStep(particles, statistics);

        if (shouldResetSelfDiffusionProcessor)
        {
            return scatterResult;
        }
        else
        {
            if (scatterResult == EquilibrationProcessingStatus::ErrorsFound || diffusionResult == EquilibrationProcessingStatus::ErrorsFound)
            {
                return EquilibrationProcessingStatus::ErrorsFound;
            }
            if (scatterResult == EquilibrationProcessingStatus::NotEnoughStatistics || diffusionResult == EquilibrationProcessingStatus::NotEnoughStatistics)
            {
                return EquilibrationProcessingStatus::NotEnoughStatistics;
            }
            return EquilibrationProcessingStatus::EnoughStatistics;
        }
    }

    void ScatterAndDiffusionProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {
        intermediateScatteringFunctionProcessor->Finish(statistics);
        selfDiffusionProcessor->Finish(statistics);
    }
}

