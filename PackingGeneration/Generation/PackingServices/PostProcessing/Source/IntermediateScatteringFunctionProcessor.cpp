// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/IntermediateScatteringFunctionProcessor.h"

#include <numeric>

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Utilities.h"
#include "Core/Headers/ScopedFile.h"

#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/LubachevsckyStillingerStep.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/DistanceServices/Headers/DistanceService.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingGenerators;

namespace PackingServices
{
    const FLOAT_TYPE IntermediateScatteringFunctionProcessor::expectedWaveVectorLength = 7.1; // As in Perez-Angel, et al, 2011, Equilibration of concentrated hard-sphere fluids

    IntermediateScatteringFunctionProcessor::IntermediateScatteringFunctionProcessor(MathService* mathService, GeometryService* geometryService,
            PackingGenerators::LubachevsckyStillingerStep* lubachevsckyStillingerStep, PackingSerializer* packingSerializer,
            const ModellingContext& context, const GenerationConfig& generationConfig)
    {
        this->context = &context;
        this->generationConfig = &generationConfig;

        this->lubachevsckyStillingerStep = lubachevsckyStillingerStep;
        this->mathService = mathService;
        this->geometryService = geometryService;
        this->packingSerializer = packingSerializer;

        FillWaveVectors();

        FillWaitingAndDecorrelationTimes();

        PrepareFiles(); // may call in Start() to overwrite files on each start
    }

    void IntermediateScatteringFunctionProcessor::PrepareFiles()
    {
        string scatteringFunctionsFolder = Path::Append(generationConfig->baseFolder, "ScatteringFunctions");
        Path::EnsureDirectory(scatteringFunctionsFolder);

        scatteringFunctionDataFilePath = Path::Append(scatteringFunctionsFolder, "scatteringFunctionData.txt");
        ScopedFile<ExceptionErrorHandler> scatteringFunctionDataFile(scatteringFunctionDataFilePath, FileOpenMode::Write);
        fprintf(scatteringFunctionDataFile, "iteration waveVectorLength scatteringFunctionValue selfPartValue referencePackingIndex\n");

        referencePackingsFolder = Path::Append(scatteringFunctionsFolder, "ReferencePackings");
        Path::EnsureDirectory(referencePackingsFolder);
    }

    void IntermediateScatteringFunctionProcessor::Start()
    {
        iterationIndex = -1;
        iterationIndexSinceReset = -1;

        waitingTimeIndexesForReferencePackings.clear();
        serialIndexesForReferencePackings.clear();
        nextSerialIndexForReferencePacking = 0;
        maxTrackedWaitingTimeIndex = -1;

        currentTime = 0;
    }

    EquilibrationProcessingStatus::Type IntermediateScatteringFunctionProcessor::ProcessStep(const Packing& particles, const MolecularDynamicsStatistics& statistics)
    {
        /////////////////
        // Preparation
        iterationIndex++;
        iterationIndexSinceReset++;
        currentTime += statistics.timePeriod;
        referencePackingAddedOnCurrentStep = false;

        /////////////////
        // Adding reference packings
        AddReferencePackingIfNecessary(particles);

        /////////////////
        // Scattering function values computation
        FLOAT_TYPE averageScatteringFunctionValue = ComputeAndSerializeScatteringFunctions(particles);

        /////////////////
        // Checking decorrelation
        RemoveDecorrelatedPackingsIfNecessary(averageScatteringFunctionValue);

        /////////////////
        // Post-processing
        EquilibrationProcessingStatus::Type result = SystemEquilibrated() ?
                EquilibrationProcessingStatus::EnoughStatistics : EquilibrationProcessingStatus::NotEnoughStatistics;

        if (result == EquilibrationProcessingStatus::EnoughStatistics)
        {
            printf("Enough statistics for alpha-relaxation time...\n");
        }

        return result;
    }

    bool IntermediateScatteringFunctionProcessor::SystemEquilibrated()
    {
        if (ShallEstimateDecorrelationByScatter())
        {
            const int expectedPackingDecorrelationsCount = 10;
            return (nextSerialIndexForReferencePacking >= (expectedPackingDecorrelationsCount + 1)); // packingDecorrelationsCount is nexSerialIndexForReferencePacking - 1
        }
        else
        {
            return FinishedEquilibrationByWaitingTimes();
        }
    }

    bool IntermediateScatteringFunctionProcessor::FinishedEquilibrationByWaitingTimes()
    {
        return (maxTrackedWaitingTimeIndex == waitingTimes.size() - 1) && waitingTimeIndexesForReferencePackings.empty();
    }

    FLOAT_TYPE IntermediateScatteringFunctionProcessor::ComputeAndSerializeScatteringFunctions(const Packing& particles)
    {
        FLOAT_TYPE lastAverageScatteringFunctionValue;
        FLOAT_TYPE lastAverageSelfPartValue;

        vector<vector<FLOAT_TYPE> > waveVectorLengthsPerReferencePacking(referencePackings.size());
        vector<vector<FLOAT_TYPE> > scatteringFunctionValuesPerReferencePacking(referencePackings.size());
        vector<vector<FLOAT_TYPE> > selfPartValuesPerReferencePacking(referencePackings.size());

        for (size_t i = 0; i < referencePackings.size(); ++i)
        {
            const Packing& referencePacking = referencePackings[i];
            vector<FLOAT_TYPE>& waveVectorLengths = waveVectorLengthsPerReferencePacking[i];
            vector<FLOAT_TYPE>& scatteringFunctionValues = scatteringFunctionValuesPerReferencePacking[i];
            vector<FLOAT_TYPE>& selfPartValues = selfPartValuesPerReferencePacking[i];
            // TODO: make distanceService compute scatteringFunctionValues for all reference packings simultaneously
            ComputeScatteringFunction(particles, referencePacking, &waveVectorLengths, &scatteringFunctionValues, &selfPartValues,
                    &lastAverageScatteringFunctionValue, &lastAverageSelfPartValue);
        }

        SerializeScatteringFunctionValues(waveVectorLengthsPerReferencePacking, scatteringFunctionValuesPerReferencePacking, selfPartValuesPerReferencePacking, serialIndexesForReferencePackings);

        // It is used only if we estimate decorrelation by scatter and then there is anyway only one reference packing
        // TODO: remove this hidden dependency, fill a vector of average values (anyway i'm returning them as a vector of vectors scatteringFunctionValuesPerReferencePacking.
        // May be just change some function signatures.
        return std::max(lastAverageScatteringFunctionValue, lastAverageSelfPartValue);
    }

    void IntermediateScatteringFunctionProcessor::SerializeScatteringFunctionValues(const vector<vector<FLOAT_TYPE> >& waveVectorLengthsPerReferencePacking,
            const vector<vector<FLOAT_TYPE> >& scatteringFunctionValuesPerReferencePacking,
            const vector<vector<FLOAT_TYPE> >& selfPartValuesPerReferencePacking,
            const vector<size_t>& referencePackingIndexes)
    {
        // Writing. TODO: move to PackingSerializer
        ScopedFile<LogErrorHandler> scatteringFunctionDataFile(scatteringFunctionDataFilePath, FileOpenMode::Append);

        for (size_t packingIndex = 0; packingIndex < waveVectorLengthsPerReferencePacking.size(); ++packingIndex)
        {
            const vector<FLOAT_TYPE>& waveVectorLengths = waveVectorLengthsPerReferencePacking[packingIndex];
            const vector<FLOAT_TYPE>& scatteringFunctionValues = scatteringFunctionValuesPerReferencePacking[packingIndex];
            const vector<FLOAT_TYPE>& selfPartValues = selfPartValuesPerReferencePacking[packingIndex];
            int referencePackingIndex = referencePackingIndexes[packingIndex];

            for (size_t waveVectorIndex = 0; waveVectorIndex < waveVectorLengths.size(); ++waveVectorIndex)
            {
                // iterationIndex is zero-based
                fprintf(scatteringFunctionDataFile, "%d %.15g %.15g %.15g %d\n",
                        iterationIndex + 1,
                        waveVectorLengths[waveVectorIndex], scatteringFunctionValues[waveVectorIndex], selfPartValues[waveVectorIndex],
                        referencePackingIndex + 1);
            }
        }
    }

    void IntermediateScatteringFunctionProcessor::ComputeScatteringFunction(const Packing& particles, const Packing& referencePacking,
            vector<FLOAT_TYPE>* waveVectorLengths, vector<FLOAT_TYPE>* scatteringFunctionValues, vector<FLOAT_TYPE>* selfPartValues,
            FLOAT_TYPE* averageScatteringFunctionValue, FLOAT_TYPE* averageSelfPartValue)
    {
        // Dirty hacks. Structure factor computation doesn't require setting services and does not require setting context and particles.
        // But i currently put it in the DistanceService, and use it not like it is supposed to be used. I also assume that the constructor and computation below do not change any of the services.
        // TODO: REWRITE!!!!
        DistanceService distanceService(mathService, geometryService, lubachevsckyStillingerStep->neighborProvider);

        std::vector<int >particleIndexesOfInterest; // default empty value. ISF will be calculated for all particles

        distanceService.FillIntermediateScatteringFunctionForWaveVectors(
            *(context->config), 
            referencePacking, 
            particles, 
            particleIndexesOfInterest,
            waveVectors, 
            scatteringFunctionValues, 
            selfPartValues);

        // No averaging. TODO: precompute waveVectorLengths
//        vector<FLOAT_TYPE> waveVectorLengths(waveVectors.size());
//        for (size_t i = 0; i < waveVectors.size(); ++i)
//        {
//            waveVectorLengths[i] = VectorUtilities::GetLength(waveVectors[i]);
//        }

        // Averaging
        // We do these averaging to write in files as little as possible.
        // Because writing values for all wave vectors at each step may consume up to 3Gb per packing during equilibration. And i have ~600 packings in a set.
        *averageScatteringFunctionValue = VectorUtilities::Sum(*scatteringFunctionValues) / scatteringFunctionValues->size();
        *averageSelfPartValue = VectorUtilities::Sum(*selfPartValues) / selfPartValues->size();

        waveVectorLengths->clear();
        waveVectorLengths->push_back(expectedWaveVectorLength);

        scatteringFunctionValues->clear();
        scatteringFunctionValues->push_back(*averageScatteringFunctionValue);

        selfPartValues->clear();
        selfPartValues->push_back(*averageSelfPartValue);
    }

    void IntermediateScatteringFunctionProcessor::AddReferencePackingIfNecessary(const Packing& particles)
    {
        if (ShallEstimateDecorrelationByScatter())
        {
            if (PackingDecorrelatedOnPreviousStepByScatter())
            {
                // TODO: merge with "if" below
                WriteReferencePacking(particles, nextSerialIndexForReferencePacking);
                AddPackingToReferencePackingsArray(particles);
                serialIndexesForReferencePackings.push_back(nextSerialIndexForReferencePacking);
                nextSerialIndexForReferencePacking++;

                referencePackingAddedOnCurrentStep = true;
            }
        }
        else
        {
            int crossedWaitingTimeIndex = GetCrossedWaitingTimeIndex();
            if (crossedWaitingTimeIndex != -1)
            {
                WriteReferencePacking(particles, nextSerialIndexForReferencePacking);
                AddPackingToReferencePackingsArray(particles);

                waitingTimeIndexesForReferencePackings.push_back(crossedWaitingTimeIndex);
                serialIndexesForReferencePackings.push_back(nextSerialIndexForReferencePacking);
                maxTrackedWaitingTimeIndex = crossedWaitingTimeIndex;

                nextSerialIndexForReferencePacking++;

                referencePackingAddedOnCurrentStep = true;
            }
        }
    }

    void IntermediateScatteringFunctionProcessor::AddPackingToReferencePackingsArray(const Packing& particles)
    {
        referencePackings.resize(referencePackings.size() + 1);

        Packing& addedReferencePacking = referencePackings[referencePackings.size() - 1];
        addedReferencePacking.resize(context->config->particlesCount);

        Particle::CopyPackingTo(particles, &addedReferencePacking);
    }

    int IntermediateScatteringFunctionProcessor::GetCrossedWaitingTimeIndex() // return nextWaitingTimeIndex
    {
        int maxWaitingTimesIndex = waitingTimes.size() - 1;

        // Can not cross any more waiting times
        if (maxTrackedWaitingTimeIndex == maxWaitingTimesIndex)
        {
            return -1;
        }

        // We ensure that we will return the last crossed waiting time, not the first one, if several have been crossed
        int firstLargerWaitingTimeIndex = -1;
        for (int i = maxTrackedWaitingTimeIndex + 1; i < waitingTimes.size(); ++i)
        {
            if (waitingTimes[i] > currentTime)
            {
                firstLargerWaitingTimeIndex = i;
                break;
            }
        }

        bool noLargerWaitingTimeFound = firstLargerWaitingTimeIndex == -1;
        int lastLowerWaitingTimeIndex = noLargerWaitingTimeFound ? maxWaitingTimesIndex : (firstLargerWaitingTimeIndex - 1);

        bool alreadyTracked = lastLowerWaitingTimeIndex == maxTrackedWaitingTimeIndex;
        if (alreadyTracked)
        {
            lastLowerWaitingTimeIndex = -1;
        }

        return lastLowerWaitingTimeIndex;
    }

    void IntermediateScatteringFunctionProcessor::RemoveDecorrelatedPackingsIfNecessary(FLOAT_TYPE averageScatteringFunctionValue)
    {
        if (ShallEstimateDecorrelationByScatter())
        {
            bool packingDecorrelated = CheckPackingDecorrelationByScatter(averageScatteringFunctionValue);
            if (packingDecorrelated)
            {
                // There is only one reference packing in this case
                referencePackings.clear();

                // TODO: pass the original packing to Start(), set a reference packing there for the first time.
                // Then can actually reset the reference packing here.
                printf("Packing decorrelated according to intermediate scattering function\n");
                iterationIndexSinceReset = -1;
                serialIndexesForReferencePackings.clear();
            }
        }
        else
        {
            RemoveDecorrelatedPackingsByDecorrelationTime();
        }
    }

    void IntermediateScatteringFunctionProcessor::RemoveDecorrelatedPackingsByDecorrelationTime()
    {
        if (waitingTimeIndexesForReferencePackings.empty())
        {
            return;
        }

        int startingIndex = waitingTimeIndexesForReferencePackings.size() - 1;
        for (int i = startingIndex; i >= 0; --i)
        {
            size_t waitingTimeIndex = waitingTimeIndexesForReferencePackings[i];
            FLOAT_TYPE waitingTime = waitingTimes[waitingTimeIndex];
            FLOAT_TYPE decorrelationTime = decorrelationTimes[waitingTimeIndex];
            FLOAT_TYPE expectedGlobalDecorrelationTime = waitingTime + decorrelationTime;

            if (expectedGlobalDecorrelationTime <= currentTime)
            {
                StlUtilities::RemoveAt(&waitingTimeIndexesForReferencePackings, i);
                StlUtilities::RemoveAt(&serialIndexesForReferencePackings, i);
                StlUtilities::RemoveAt(&referencePackings, i);
            }
        }
    }

    void IntermediateScatteringFunctionProcessor::WriteReferencePacking(const Model::Packing& particles, int referencePackingIndex)
    {
        // Waiting times can be stored from log and iterationStatistics.txt
        string referencePackingName = "packing_reference_" + Utilities::ConvertToString(referencePackingIndex + 1) + ".xyzd";
        string referencePackingPath = Path::Append(referencePackingsFolder, referencePackingName);
        packingSerializer->SerializePacking(referencePackingPath, particles);
    }

    bool IntermediateScatteringFunctionProcessor::CheckPackingDecorrelationByScatter(FLOAT_TYPE averageScatteringFunctionValue)
    {
        // TODO: refactor the class. The logic is mixed somehow
        if (PackingDecorrelatedOnPreviousStepByScatter())
        {
            lastStructureFactorValue = averageScatteringFunctionValue;
        }

        FLOAT_TYPE normalizedScatteringFunctionValue = averageScatteringFunctionValue / lastStructureFactorValue;
        size_t index = iterationIndexSinceReset % scatteringFunctionAveragingSize;
        lastNormalizedScatteringFunctionValues[index] = normalizedScatteringFunctionValue;

        bool packingDecorrelated = false;
        if (static_cast<size_t>(iterationIndexSinceReset + 1) >= scatteringFunctionAveragingSize)
        {
            FLOAT_TYPE averageLastNormalizedScatteringValue = VectorUtilities::Sum(lastNormalizedScatteringFunctionValues) / scatteringFunctionAveragingSize;

            // alphaRelaxationValue = B * exp(t / t_alpha).
            // If t = t_alpha, alphaRelaxationValue = B. In Brambilla (2009): B = 0.68.
            // Sometimes i determine t_alpha by checking t where alphaRelaxationValue = 0.68.
            const FLOAT_TYPE alphaRelaxationValue = 0.68 * std::exp(-1);
//            const FLOAT_TYPE alphaRelaxationValue = 10; // debug
            const FLOAT_TYPE threshold = 0.1;
            packingDecorrelated = averageLastNormalizedScatteringValue < alphaRelaxationValue - threshold;
        }

        return packingDecorrelated;
    }

    bool IntermediateScatteringFunctionProcessor::ShallEstimateDecorrelationByScatter()
    {
        return waitingTimes.empty();
    }

    bool IntermediateScatteringFunctionProcessor::PackingDecorrelatedOnPreviousStepByScatter()
    {
        return iterationIndexSinceReset == 0;
    }

    // This method is used to make the Self-Diffusion computation change the reference packing synchronously.
    // This will work if we add a packing after the previous one is completely decorrelated, otherwise self-diffusion does not proceed well
    bool IntermediateScatteringFunctionProcessor::ReferencePackingAddedOnCurrentStep()
    {
        return referencePackingAddedOnCurrentStep;
    }

    void IntermediateScatteringFunctionProcessor::Finish(Model::MolecularDynamicsStatistics* statistics)
    {

    }

    void IntermediateScatteringFunctionProcessor::FillWaitingAndDecorrelationTimes()
    {
        string waitingTimesPath = Path::Append(generationConfig->baseFolder, "waitingAndDecorrelationTimes.txt");
        if (!Path::Exists(waitingTimesPath))
        {
            return;
        }

        // TODO: move to PackingSerializer
        waitingTimes.clear();
        decorrelationTimes.clear();

        ScopedFile<ExceptionErrorHandler> file(waitingTimesPath, FileOpenMode::Read);
        fscanf(file, "waitingTimes decorrelationTimes\n"); // TODO: skip the first line correctly
        while(true)
        {
            FLOAT_TYPE waitingTime;
            FLOAT_TYPE decorrelationTime;
            int result = fscanf(file, FLOAT_FORMAT " " FLOAT_FORMAT "\n", &waitingTime, &decorrelationTime);

            bool endOfFile = result == EOF;
            if (endOfFile)
            {
                return;
            }
            waitingTimes.push_back(waitingTime);
            decorrelationTimes.push_back(decorrelationTime);
        }
    }

    void IntermediateScatteringFunctionProcessor::FillWaveVectors()
    {
        DistanceService distanceService(mathService, geometryService, lubachevsckyStillingerStep->neighborProvider); // A dirty hack!
        FLOAT_TYPE waveVectorHalfWidth = 0.2; // Packing is periodic, waveVector can accept only countable values, not necessarily equal to 7.1

        vector<SpatialVector> waveVectorCandidates;
        vector<FLOAT_TYPE> waveVectorLengths;
        distanceService.FillPeriodicWaveVectors(*(context->config), expectedWaveVectorLength, waveVectorHalfWidth, &waveVectorCandidates, &waveVectorLengths);

        // Find necessary wave vectors

        vector<FLOAT_TYPE> waveVectorLengthDifferences(waveVectorLengths.size());

        VectorUtilities::SubtractValue(waveVectorLengths, expectedWaveVectorLength, &waveVectorLengthDifferences);
        VectorUtilities::Abs(waveVectorLengthDifferences, &waveVectorLengthDifferences);

        // Sort wave vector lengths by difference to the expectedWaveVectorLength (find permutation)
        vector<int> permutation;
        StlUtilities::SortPermutation(waveVectorLengthDifferences, &permutation);

        // Find the last index of sorted wave vectors that has the Nth unique length
        int maxUniqueLengths = 4;
        int uniqueLengthsCount = 0;
        size_t neededVectorsCount = 0;
        FLOAT_TYPE previousLength = -1;
        for (size_t i = 0; i < waveVectorLengths.size(); ++i)
        {
            size_t sortedIndex = permutation[i];
            FLOAT_TYPE currentLength = waveVectorLengths[sortedIndex];

            if (currentLength != previousLength)
            {
                previousLength = currentLength;
                uniqueLengthsCount++;

                if (uniqueLengthsCount > maxUniqueLengths)
                {
                    neededVectorsCount = i;
                    break;
                }
            }
        }

        // Use all the wave vectors up to this index
        waveVectors.resize(neededVectorsCount);
        for (size_t i = 0; i < neededVectorsCount; ++i)
        {
            size_t sortedIndex = permutation[i];
            waveVectors[i] = waveVectorCandidates[sortedIndex];
        }
    }
}

