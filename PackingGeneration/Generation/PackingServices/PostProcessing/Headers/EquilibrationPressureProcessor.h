// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_EquilibrationPressureProcessor_h
#define Generation_PackingServices_PostProcessing_Headers_EquilibrationPressureProcessor_h

#include "Generation/PackingServices/PostProcessing/Headers/IEquilibrationStatisticsGatherer.h"

namespace PackingServices
{
    class EquilibrationPressureProcessor : public virtual IEquilibrationStatisticsGatherer
    {
    private:
        int averagingWindowWidth;
        std::vector<Core::FLOAT_TYPE> pressures;

        Core::FLOAT_TYPE maxRelativeChange;

        int iterationIndex;

    public:
        explicit EquilibrationPressureProcessor(int eventsPerParticle);

        void Start();

        EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics);

        void Finish(Model::MolecularDynamicsStatistics* statistics);

    private:
        void GetPressuresSafe(int currentPressuresLastInclusiveIndex, Core::FLOAT_TYPE* previousPressure, Core::FLOAT_TYPE* currentPressure);

        Core::FLOAT_TYPE GetPressureAverage(int pressuresFirstIndex, int pressuresLastInclusiveIndex,
                int splitPressuresFirstIndex, int splitPressuresLastInclusiveIndex);

        void FillCyclicArrayRangesByLastIndex(int cyclicArraySize, int batchSize, int batchLastInclusiveIndex,
                int* batchFirstIndex, int* splitBatchFirstIndex, int* splitBatchLastInclusiveIndex);

        void FillCyclicArrayRangesByFirstIndex(int cyclicArraySize, int batchSize, int batchFirstIndex,
                int* batchLastInclusiveIndex, int* splitBatchFirstIndex, int* splitBatchLastInclusiveIndex);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_EquilibrationPressureProcessor_h */

