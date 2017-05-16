// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ErrorRateProcessor.h"

#include <cstdio>
#include "Core/Headers/VectorUtilities.h"

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    void ErrorRateProcessor::Start()
    {
        iterationIndex = -1;
    }

    EquilibrationProcessingStatus::Type ErrorRateProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        iterationIndex++;

        int errorExisted = statistics.collisionErrorsExisted ? 1 : 0;
        size_t errorsExistedMaskIndex = iterationIndex % averagingWindowWidth;
        errorsExistedMask[errorsExistedMaskIndex] = errorExisted;

        // I can use any other check interval
        int checkInterval = averagingWindowWidth;

        if (iterationIndex < (averagingWindowWidth - 1) || iterationIndex % checkInterval != 0)
        {
            return EquilibrationProcessingStatus::EnoughStatistics;
        }

        int errorsCount = VectorUtilities::Sum(errorsExistedMask);

        const FLOAT_TYPE maxErrorFrequency = 0.5;
        FLOAT_TYPE errorFrequency = static_cast<FLOAT_TYPE>(errorsCount) / averagingWindowWidth;
        bool errorFrequencyHigh= errorFrequency > maxErrorFrequency;
        if (errorFrequencyHigh)
        {
            printf("WARNING: frequency of cycles with errors is %f and is > %f. Terminating...\n", errorFrequency, maxErrorFrequency);
        }

        EquilibrationProcessingStatus::Type result = errorFrequencyHigh ? EquilibrationProcessingStatus::ErrorsFound : EquilibrationProcessingStatus::EnoughStatistics;
        return result;
    }

    void ErrorRateProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {

    }
}

