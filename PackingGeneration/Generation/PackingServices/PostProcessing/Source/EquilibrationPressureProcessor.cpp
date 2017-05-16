// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/EquilibrationPressureProcessor.h"

#include <numeric>
#include <cstdio>
#include <cmath>

using namespace std;
using namespace Core;
using namespace Model;

namespace PackingServices
{
    EquilibrationPressureProcessor::EquilibrationPressureProcessor(int eventsPerParticle)
    {
//        // Settings used for the RCP, phase space structure, and phi_g prediction papers were: averagingWindowWidth = 100 for 20 events per particle, maxRelativeChange = 1e-4
//        averagingWindowWidth = 100 * 20 / eventsPerParticle;
//        maxRelativeChange = 1e-4; // RCP, phase space structure, and phi_g prediction papers

        // Only structure factor investigation. Do not care about pressure.
        averagingWindowWidth = std::ceil(5 * 20 / (double)eventsPerParticle);
        maxRelativeChange = 0.05;

//        // Debug
//        averagingWindowWidth = 5;
//        maxRelativeChange = 1;

        pressures.resize(2 * averagingWindowWidth);
    }

    void EquilibrationPressureProcessor::Start()
    {
        iterationIndex = -1;
    }

    EquilibrationProcessingStatus::Type EquilibrationPressureProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        iterationIndex++;

        int currentPressuresLastInclusiveIndex = iterationIndex % pressures.size();

        pressures[currentPressuresLastInclusiveIndex] = statistics.reducedPressure;

        if (static_cast<size_t>(iterationIndex) < pressures.size() - 1)
        {
            return EquilibrationProcessingStatus::NotEnoughStatistics;
        }

        if (iterationIndex % averagingWindowWidth != 0)
        {
            return EquilibrationProcessingStatus::NotEnoughStatistics;
        }

        FLOAT_TYPE previousPressure;
        FLOAT_TYPE currentPressure;
        GetPressuresSafe(currentPressuresLastInclusiveIndex, &previousPressure, &currentPressure);

        FLOAT_TYPE relativeChange = std::abs(currentPressure - previousPressure) / ( (previousPressure + currentPressure) * 0.5 );
        printf("Relative pressure change: %f\n", relativeChange);

        if (relativeChange < maxRelativeChange)
        {
            printf("Relative pressure change is < %g. Enough statistics for pressure...\n", maxRelativeChange);
            return EquilibrationProcessingStatus::EnoughStatistics;
        }
        else
        {
            return EquilibrationProcessingStatus::NotEnoughStatistics;
        }
    }

    void EquilibrationPressureProcessor::GetPressuresSafe(int currentPressuresLastInclusiveIndex, FLOAT_TYPE* previousPressure, FLOAT_TYPE* currentPressure)
    {
        // Previous values and current values can have at most two intervals each in the pressures array
        int currentPressuresFirstIndex = -1;
        int splitCurrentPressuresFirstIndex = -1;
        int splitCurrentPressuresLastInclusiveIndex = -1;
        FillCyclicArrayRangesByLastIndex(pressures.size(), averagingWindowWidth, currentPressuresLastInclusiveIndex,
                &currentPressuresFirstIndex, &splitCurrentPressuresFirstIndex, &splitCurrentPressuresLastInclusiveIndex);

        int previousPressuresFirstIndex = currentPressuresLastInclusiveIndex + 1;
        if (static_cast<size_t>(previousPressuresFirstIndex) == pressures.size())
        {
            previousPressuresFirstIndex = 0;
        }
        int previousPressuresLastInclusiveIndex = -1;
        int splitPreviousPressuresFirstIndex = -1;
        int splitPreviousPressuresLastInclusiveIndex = -1;

        FillCyclicArrayRangesByFirstIndex(pressures.size(), averagingWindowWidth, previousPressuresFirstIndex,
                    &previousPressuresLastInclusiveIndex, &splitPreviousPressuresFirstIndex, &splitPreviousPressuresLastInclusiveIndex);

        *previousPressure = GetPressureAverage(previousPressuresFirstIndex, previousPressuresLastInclusiveIndex,
                splitPreviousPressuresFirstIndex, splitPreviousPressuresLastInclusiveIndex);

        *currentPressure = GetPressureAverage(currentPressuresFirstIndex, currentPressuresLastInclusiveIndex,
                splitCurrentPressuresFirstIndex, splitCurrentPressuresLastInclusiveIndex);
    }

    FLOAT_TYPE EquilibrationPressureProcessor::GetPressureAverage(int pressuresFirstIndex, int pressuresLastInclusiveIndex,
            int splitPressuresFirstIndex, int splitPressuresLastInclusiveIndex)
    {
        FLOAT_TYPE pressuresSum = accumulate(pressures.begin() + pressuresFirstIndex, pressures.begin() + pressuresLastInclusiveIndex + 1, 0.0);
        if (splitPressuresFirstIndex != -1)
        {
            pressuresSum += accumulate(pressures.begin() + splitPressuresFirstIndex, pressures.begin() + splitPressuresLastInclusiveIndex + 1, 0.0);
        }

        return pressuresSum / averagingWindowWidth;
    }

    void EquilibrationPressureProcessor::FillCyclicArrayRangesByLastIndex(int cyclicArraySize, int batchSize, int batchLastInclusiveIndex,
            int* batchFirstIndex, int* splitBatchFirstIndex, int* splitBatchLastInclusiveIndex)
    {
        *batchFirstIndex = batchLastInclusiveIndex - batchSize + 1;
        *splitBatchFirstIndex = -1;
        *splitBatchLastInclusiveIndex = -1;

        // Current pressures are at the end of the cyclic array with pressures
        if (*batchFirstIndex < 0)
        {
            *splitBatchFirstIndex = *batchFirstIndex + cyclicArraySize;
            *splitBatchLastInclusiveIndex = cyclicArraySize - 1;
            *batchFirstIndex = 0;
        }
    }

    void EquilibrationPressureProcessor::FillCyclicArrayRangesByFirstIndex(int cyclicArraySize, int batchSize, int batchFirstIndex,
            int* batchLastInclusiveIndex, int* splitBatchFirstIndex, int* splitBatchLastInclusiveIndex)
    {
        *batchLastInclusiveIndex = batchFirstIndex + batchSize - 1;
        *splitBatchFirstIndex = -1;
        *splitBatchLastInclusiveIndex = -1;

        if (*batchLastInclusiveIndex >= cyclicArraySize)
        {
            *splitBatchFirstIndex = 0;
            *splitBatchLastInclusiveIndex = *batchLastInclusiveIndex - cyclicArraySize;
            *batchLastInclusiveIndex = cyclicArraySize - 1;
        }
    }

    void EquilibrationPressureProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {
        int currentPressuresLastInclusiveIndex = iterationIndex % pressures.size();

        if (static_cast<size_t>(iterationIndex) < pressures.size() - 1)
        {
            return;
        }

        FLOAT_TYPE previousPressure;
        FLOAT_TYPE currentPressure;
        GetPressuresSafe(currentPressuresLastInclusiveIndex, &previousPressure, &currentPressure);

        statistics->reducedPressure = currentPressure;
    }
}

