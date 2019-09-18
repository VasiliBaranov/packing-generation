// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_PackingSerializer_h
#define Generation_PackingServices_Headers_PackingSerializer_h

#include <stdio.h>

#include "Generation/Model/Headers/Types.h"
#include "Generation/Constants.h"
#include "Core/Headers/IEndiannessProvider.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/Utilities.h"
#include "Generation/PackingServices/PostProcessing/Headers/OrderService.h"
namespace Model { class ExecutionConfig; }

namespace PackingServices
{
    // Implements basic serialization methods.
    // Don't mark input args as const, as they are read or written in little endian, and can be therefore updated (or would imply copy constructors).
    class PackingSerializer
    {
    private:
        Core::IEndiannessProvider* endiannessProvider;

    public:
        explicit PackingSerializer(Core::IEndiannessProvider* endiannessProvider);

        // Reads
        void ReadPacking(std::string packingFilePath, Model::Packing* particles) const;

        void ReadConfig(std::string baseFolder, Model::ExecutionConfig* config) const;

        // TODO: create an ActiveConfig class, with shift, but without particle count
        void ReadActiveConfig(std::string activeConfigPath, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) const;

        void ReadParticleDiameters(std::string diametersFilePath, Model::Packing* particles) const;

        void ReadInsertionRadii(std::string radiiFilePath, std::vector<Core::FLOAT_TYPE>* insertionRadii) const;

        void ReadPackingInfo(std::string infoFilePath, Model::PackingInfo* packingInfo) const;

        void ReadParticleDistances(std::string distancesFilePath, int particleCount, Core::FLOAT_TYPE** particleDistances) const;

        void ReadContractionEnergies(std::string energiesFilePath, std::vector<Core::FLOAT_TYPE>* contractionRatios,
                std::vector<Core::FLOAT_TYPE>* energyPowers, std::vector<Core::FLOAT_TYPE>* contractionEnergies, std::vector<int>* nonRattlersCounts) const;

        void ReadNearestNeighbors(std::string neighborsFilePath, std::vector<Model::ParticlePair>* closestPairs) const;

        void ReadImmobileParticleIndexes(std::string immobileParticlesPath, std::vector<Model::ParticleIndex>* immobileParticleIndexes) const;

        // Writes
        void AppendPacking(std::string packingFilePath, const Model::Packing& particles) const;

        void SerializePackingInfo(std::string infoFilePath, const Model::SystemConfig& config, const Model::PackingInfo& packingInfo) const;

        void SerializePacking(std::string packingFilePath, const Model::Packing& particles) const;

        void SerializeConfig(std::string configFilePath, const Model::ExecutionConfig& config) const;

        void SerializeActiveConfig(std::string activeConfigPath, const Model::SystemConfig& activeConfig, const Core::SpatialVector& shift) const;

        void SerializeInsertionRadii(std::string radiiFilePath, const std::vector<Core::FLOAT_TYPE>& insertionRadii) const;

        void SerializeDistancesToSurfaces(std::string distancesFolderPath, const std::vector<int>& surfaceIndexes, const std::vector<std::vector<Core::FLOAT_TYPE> >& distancesToSurfaces, bool shouldAppend) const;

        void SerializeContactNumberDistribution(std::string contactNumberDistributionFilePath, const std::vector<int>& neighborCounts, const std::vector<int>& neighborCountFrequencies) const;

        void SerializeContactingNeighborIndexes(std::string contactingNeighborIndexesFilePath, const std::vector<std::vector<int>>& neighborIndexes) const;

        void SerializeContactingNeighborDistances(std::string contactingNeighborDistancesFilePath, const std::vector<Core::FLOAT_TYPE>& contactingNeighborDistances) const;

        void SerializeParticleDirections(std::string distancesFilePath, int particleCount, const std::vector<OrderService::NeighborDirections>& particleDirections) const;

        void SerializeContractionEnergies(std::string energiesFilePath, const std::vector<Core::FLOAT_TYPE>& contractionRatios,
                const std::vector<Core::FLOAT_TYPE>& energyPowers, const std::vector<Core::FLOAT_TYPE>& contractionEnergies, const std::vector<int>& nonRattlersCounts) const;

        void SerializeOrder(std::string orderFilePath, const OrderService::Order& order) const;

        void SerializeEntropy(std::string entropyFilePath, Core::FLOAT_TYPE entropy) const;

        void SerializeLocalEntropy(std::string entropyFilePath, Core::FLOAT_TYPE entropy, const std::vector<Core::FLOAT_TYPE>& localEntropies) const;

        void SerializeHessianEigenvalues(std::string eigenvaluesFilePath, const std::vector<Core::FLOAT_TYPE>& hessianEigenvalues) const;

        void SerializeMatrix(std::string filePath, const Core::FLOAT_TYPE** matrix, int dimension) const;

        void SerializePressures(std::string pressuresFilePath, const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<Core::FLOAT_TYPE>& energyPowers, const std::vector<Core::FLOAT_TYPE>& pressures) const;

        void SerializeMolecularDynamicsStatistics(std::string statisticsFilePath, const Model::MolecularDynamicsStatistics& statistics) const;

        void SerializeMolecularDynamicsStatistics(std::string statisticsFilePath, const std::vector<Model::PressureData>& pressuresData) const;

        void SerializePairCorrelationFunction(std::string pairCorrelationFunctionFilePath, const Model::PairCorrelationFunction& pairCorrelationFunction) const;

        void SerializeStructureFactor(std::string structureFactorFilePath, const Model::StructureFactor& structureFactor) const;

        void SerializeLocalOrientationalDisorder(std::string disorderFilePath, const OrderService::LocalOrientationalDisorder& disorder) const;

        void SerializeCloseNeighbors(std::string neighborsFilePath, const OrderService::LocalOrientationalDisorder& disorder) const;

        void SerializeNeighborVectorSums(std::string neighborVectorSumsFilePath, const std::vector<Core::FLOAT_TYPE>& neighborVectorSumsNorms, const std::vector<bool>& rattlerMask, Core::FLOAT_TYPE maxNeighborVectorSumForNonRattlers) const;

        void SerializeNearestNeighbors(std::string nearestNeighborsFilePath, const std::vector<Model::ParticlePair>& closestPairs, const std::vector<bool>& isImmobileMask) const;

        virtual ~PackingSerializer();

    private:
        void WritePacking(std::string packingFilePath, const Model::Packing& particles, bool shouldAppend) const;

        boost::array<Core::FLOAT_TYPE, 3> MakeSpatialVectorThreeDimensional(const Core::SpatialVector& vector) const;

        DISALLOW_COPY_AND_ASSIGN(PackingSerializer);

        // Though one may need to read float values into a structure, so passing void* instead of T* makes sense,
        // i prefer to pass T* for type safety and to make casts explicit.
        template<class T>
        void SwapBytesIfNecessary(const T* source, T* target, int valuesCount) const
        {
            if (endiannessProvider->IsBigEndian())
            {
                for (int i = 0; i < valuesCount; ++i)
                {
                    target[i] = Core::Utilities::DoByteSwap(source[i]);
                }
            }
            else
            {
                // Can't pass TRandomAccessIteratorSource, TRandomAccessIteratorTarget to this function and pass vector.begin(), if necessary,
                // because will not be able to perform this comparison.
                if (source != target)
                {
                    std::copy(source, source + valuesCount, target);
                }
            }
        }

        template<class T>
        void WriteLittleEndian(const T* values, size_t valuesCount, FILE* file) const
        {
            std::vector<T> valuesCopy(valuesCount);
            T* valuesCopyArray = &valuesCopy[0];
            SwapBytesIfNecessary(values, valuesCopyArray, valuesCount);

            fwrite(valuesCopyArray, sizeof(T), valuesCount, file);
            fflush(file); // if we do not flush and valuesCopy is deleted, nothing may be written to the file
        }

        // Not a generic container to be sure that elements are in consecutive order
        template<class T>
        void WriteLittleEndian(const std::vector<T>& values, FILE* file) const
        {
            // This is a hack, but it's the most popular answer in http://stackoverflow.com/a/1693505/2029962
            // "Fastest way to write large STL vector to file using STL".
            WriteLittleEndian<T>(&values[0], values.size(), file);
        }

        template<class T>
        bool ReadLittleEndian(T* values, size_t valuesCount, FILE* file) const
        {
            size_t readValuesCount = fread(values, sizeof(T), valuesCount, file);
            SwapBytesIfNecessary(values, values, valuesCount);
            return readValuesCount == valuesCount;
        }
    };
}

#endif /* Generation_PackingServices_Headers_PackingSerializer_h */

