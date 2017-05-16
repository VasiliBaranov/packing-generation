// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_ErrorRateProcessor_h
#define Generation_PackingServices_PostProcessing_Headers_ErrorRateProcessor_h

#include "Generation/PackingServices/PostProcessing/Headers/IEquilibrationStatisticsGatherer.h"

namespace PackingServices
{
    class ErrorRateProcessor : public virtual IEquilibrationStatisticsGatherer
    {
    private:
        static const int averagingWindowWidth = 100;
        boost::array<int, averagingWindowWidth> errorsExistedMask;

        int iterationIndex;

    public:
        void Start();

        EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics);

        void Finish(Model::MolecularDynamicsStatistics* statistics);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_ErrorRateProcessor_h */

