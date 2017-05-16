// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_ScatterAndDiffusionProcessor_h
#define Generation_PackingServices_PostProcessing_Headers_ScatterAndDiffusionProcessor_h

#include "Generation/PackingServices/PostProcessing/Headers/IEquilibrationStatisticsGatherer.h"

namespace PackingServices { class SelfDiffusionProcessor; }
namespace PackingServices { class IntermediateScatteringFunctionProcessor; }

namespace PackingServices
{
    class ScatterAndDiffusionProcessor : public virtual IEquilibrationStatisticsGatherer
    {
    private:
        SelfDiffusionProcessor* selfDiffusionProcessor;
        IntermediateScatteringFunctionProcessor* intermediateScatteringFunctionProcessor;

        bool shouldResetSelfDiffusionProcessor;

    public:
        ScatterAndDiffusionProcessor(IntermediateScatteringFunctionProcessor* intermediateScatteringFunctionProcessor, SelfDiffusionProcessor* selfDiffusionProcessor);

        void Start();

        EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics);

        void Finish(Model::MolecularDynamicsStatistics* statistics);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_ScatterAndDiffusionProcessor_h */

