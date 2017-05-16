// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_SelfDiffusionProcessor_h
#define Generation_PackingServices_PostProcessing_Headers_SelfDiffusionProcessor_h

#include <string>
#include "Generation/PackingServices/PostProcessing/Headers/IEquilibrationStatisticsGatherer.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"

namespace PackingGenerators { class LubachevsckyStillingerStep; }
namespace PackingServices { class MathService; }
namespace PackingServices { struct PackingSerializer; }

namespace PackingServices
{
    class SelfDiffusionProcessor : public virtual IEquilibrationStatisticsGatherer
    {
    private:
        MathService* mathService;
        PackingSerializer* packingSerializer;
        PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep;

        std::string equilibratedPackingPath;

        int iterationIndex;
        int iterationIndexSinceReset;

        Model::Packing originalPacking;
        Model::Packing referencePacking;

        std::vector<PackingGenerators::CollidingPair> initialCollidedPairs;

        static const bool resetsExternally = true;
        static const int diffusionCalculationPeriod = 1000; // not important if resetsExternally = true
        static const int packingWritePeriod = 1000;

    public:
        SelfDiffusionProcessor(std::string equilibratedPackingPath, MathService* mathService, PackingSerializer* packingSerializer,
                PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep);

        void Start();

        EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics);

        void Finish(Model::MolecularDynamicsStatistics* statistics);

        void ResetReferencePacking();

        bool ResetsExternally();

    private:
        Core::FLOAT_TYPE GetDistanceBetweenPackings(const Model::Packing& firstPacking, const Model::Packing& secondPacking) const;

        void WritePackingDifference(const Model::Packing& firstPacking, const Model::Packing& secondPacking, std::string filePath) const;

        void WriteCollidedPairs(const std::vector<PackingGenerators::CollidingPair>& collidedPairs, std::string filePath) const;

        bool WriteCurrentState(const Model::Packing& originalPacking, const Model::Packing& referencePacking, const Model::Packing& currentPacking, int stateIndex, bool collisionErrorsExisted) const;

        void WriteCollidedPairs(const std::vector<PackingGenerators::CollidingPair>& initialCollidedPairs, const std::vector<PackingGenerators::CollidingPair>& collidedPairs, Model::ParticleIndex particlesCount, int stateIndex) const;

    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_SelfDiffusionProcessor_h */

