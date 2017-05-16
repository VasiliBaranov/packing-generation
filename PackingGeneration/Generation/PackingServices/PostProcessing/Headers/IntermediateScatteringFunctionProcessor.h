// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_IntermediateScatteringFunctionProcessor_h
#define Generation_PackingServices_PostProcessing_Headers_IntermediateScatteringFunctionProcessor_h

#include "Generation/PackingServices/PostProcessing/Headers/IEquilibrationStatisticsGatherer.h"

namespace PackingGenerators { class LubachevsckyStillingerStep; }
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { struct PackingSerializer; }

namespace Model { class ModellingContext; }
namespace Model { class GenerationConfig; }

namespace PackingServices
{
    class IntermediateScatteringFunctionProcessor : public virtual IEquilibrationStatisticsGatherer
    {
    private:
        // Services
        PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep;
        MathService* mathService;
        GeometryService* geometryService;
        PackingSerializer* packingSerializer;

        // TODO: make refs
        const Model::ModellingContext* context;
        const Model::GenerationConfig* generationConfig;

        // Data
        int iterationIndex;
        int iterationIndexSinceReset;
//        int packingDecorrelationsCount;

        std::vector<Model::Packing> referencePackings;

        std::vector<Core::SpatialVector> waveVectors;

        Core::FLOAT_TYPE currentTime;

        // Predefined waiting and decorrelation times
        std::vector<Core::FLOAT_TYPE> waitingTimes;
        std::vector<Core::FLOAT_TYPE> decorrelationTimes;
        std::vector<size_t> waitingTimeIndexesForReferencePackings;
        std::vector<size_t> serialIndexesForReferencePackings; // needed if time step is so large that we skip several waiting times
        size_t nextSerialIndexForReferencePacking; // needed if the last packing is decorrelated and removed from serialIndexesForReferencePackings before we add another reference packing
        int maxTrackedWaitingTimeIndex; // needed if the last packing is decorrelated and removed from waitingTimeIndexesForReferencePackings
        // end

        // Manual estimation of equilibration
        Core::FLOAT_TYPE lastStructureFactorValue;
        static const size_t scatteringFunctionAveragingSize = 10;
//        static const size_t scatteringFunctionAveragingSize = 2; // debug
        boost::array<Core::FLOAT_TYPE, scatteringFunctionAveragingSize> lastNormalizedScatteringFunctionValues;
        // end

        bool referencePackingAddedOnCurrentStep;

        static const Core::FLOAT_TYPE expectedWaveVectorLength;

        // File paths
        std::string scatteringFunctionDataFilePath;

        std::string waitingTimesFolder;
        std::string referencePackingsFolder;

    public:
        IntermediateScatteringFunctionProcessor(MathService* mathService, GeometryService* geometryService,
                PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer,
                const Model::ModellingContext& context, const Model::GenerationConfig& generationConfig);

        void Start();

        EquilibrationProcessingStatus::Type ProcessStep(const Model::Packing& particles, const Model::MolecularDynamicsStatistics& statistics);

        void Finish(Model::MolecularDynamicsStatistics* statistics);

        bool ReferencePackingAddedOnCurrentStep();

    private:
        Core::FLOAT_TYPE ComputeAndSerializeScatteringFunctions(const Model::Packing& particles);

        void ComputeScatteringFunction(const Model::Packing& particles, const Model::Packing& referencePacking,
                std::vector<Core::FLOAT_TYPE>* waveVectorLengths, std::vector<Core::FLOAT_TYPE>* scatteringFunctionValues, std::vector<Core::FLOAT_TYPE>* selfPartValues,
                Core::FLOAT_TYPE* averageScatteringFunctionValue, Core::FLOAT_TYPE* averageSelfPartValue);

        bool SystemEquilibrated();

        bool FinishedEquilibrationByWaitingTimes();

        void AddReferencePackingIfNecessary(const Model::Packing& particles);

        void AddPackingToReferencePackingsArray(const Model::Packing& particles);

        bool CheckPackingDecorrelationByScatter(Core::FLOAT_TYPE averageScatteringFunctionValue);

        int GetCrossedWaitingTimeIndex();

        void RemoveDecorrelatedPackingsIfNecessary(Core::FLOAT_TYPE averageScatteringFunctionValue);

        void RemoveDecorrelatedPackingsByDecorrelationTime();

        void WriteReferencePacking(const Model::Packing& particles, int referencePackingIndex);

        void FillWaveVectors();

        void FillWaitingAndDecorrelationTimes();

        void PrepareFiles();

        void SerializeScatteringFunctionValues(const std::vector<std::vector<Core::FLOAT_TYPE> >& waveVectorLengthsPerReferencePacking,
                const std::vector<std::vector<Core::FLOAT_TYPE> >& scatteringFunctionValuesPerReferencePacking,
                const std::vector<std::vector<Core::FLOAT_TYPE> >& selfPartValuesPerReferencePacking,
                const std::vector<size_t>& referencePackingIndexes);

        bool ShallEstimateDecorrelationByScatter();

        bool PackingDecorrelatedOnPreviousStepByScatter();
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_IntermediateScatteringFunctionProcessor_h */

