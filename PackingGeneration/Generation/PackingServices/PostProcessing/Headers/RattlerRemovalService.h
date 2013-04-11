// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_RattlerRemovalService_h
#define Generation_PackingServices_PostProcessing_Headers_RattlerRemovalService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"
namespace Model { class ExecutionConfig; }

namespace PackingServices
{
    class RattlerRemovalService : public BaseDistanceService
    {
    public:
        int minNeighborsCount;

    public:
        RattlerRemovalService(MathService* mathService, INeighborProvider* neighborProvider);

        void SetParticles(const Model::Packing& particles);

        int GetMinNeighborsCount() const;

        void SetMinNeighborsCount(int value);

        // NOTE: i think that here it's ok to use vector<bool>
        void FillRattlerMask(Core::FLOAT_TYPE contractionRatio, std::vector<bool>* rattlerMask) const;

        int FindNonRattlersCount(const std::vector<bool>& rattlerMask) const;

        void FillNonRattlerPacking(const std::vector<bool>& rattlerMask, Model::Packing* nonRattlerParticles) const;

        void FillNonRattlerConfig(int nonRattlersCount, const Model::ExecutionConfig& oldConfig, Model::ExecutionConfig* newConfig) const;

        void FillNonRattlerPackingInfo(int nonRattlersCount, const Model::Packing& nonRattlerParticles, const Model::ExecutionConfig& newConfig, const Model::PackingInfo& oldInfo, Model::PackingInfo* newInfo) const;

    private:
        void CheckIfRattler(Model::ParticleIndex particleIndex, Core::FLOAT_TYPE contractionRatio, std::vector<bool>* rattlerMask, std::vector<bool>* processedMask) const;

        DISALLOW_COPY_AND_ASSIGN(RattlerRemovalService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_RattlerRemovalService_h */

