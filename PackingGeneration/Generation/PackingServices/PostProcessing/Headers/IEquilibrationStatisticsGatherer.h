// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_IEquilibrationStatisticsGatherer_h
#define Generation_PackingServices_PostProcessing_Headers_IEquilibrationStatisticsGatherer_h

#include "Generation/Model/Headers/Types.h"

//namespace Model { class Packing; }
namespace Model { class MolecularDynamicsStatistics; }

namespace PackingServices
{
    // CalculateStationaryStatistics processors
    struct EquilibrationProcessingStatus
    {
        enum Type
        {
            ErrorsFound = 0,
            NotEnoughStatistics = 1,
            EnoughStatistics = 2,
        };
    };

    class IEquilibrationStatisticsGatherer
    {
    public:
        virtual void Start() = 0;

        virtual EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics) = 0;

        virtual void Finish(Model::MolecularDynamicsStatistics* statistics) = 0;

        virtual ~IEquilibrationStatisticsGatherer(){ };
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_IEquilibrationStatisticsGatherer_h */

