// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MinIterationsProcessor.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    MinIterationsProcessor::MinIterationsProcessor(int minIterationsCount)
    {
        this->minIterationsCount = minIterationsCount;
    }

    void MinIterationsProcessor::Start()
    {
        iterationIndex = -1;
    }

    EquilibrationProcessingStatus::Type MinIterationsProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        iterationIndex++;

        EquilibrationProcessingStatus::Type result = ((iterationIndex + 1) >= minIterationsCount) ?
                EquilibrationProcessingStatus::EnoughStatistics : EquilibrationProcessingStatus::NotEnoughStatistics;

        return result;
    }

    void MinIterationsProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {

    }
}

