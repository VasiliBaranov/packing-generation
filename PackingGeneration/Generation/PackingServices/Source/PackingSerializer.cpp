// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingSerializer.h"

#include "Core/Headers/Path.h"
#include "Core/Headers/ScopedFile.h"
#include "Core/Headers/IEndiannessProvider.h"
#include "Generation/Model/Headers/Config.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace Generation;

namespace PackingServices
{
    PackingSerializer::PackingSerializer(IEndiannessProvider* endiannessProvider)
    {
        this->endiannessProvider = endiannessProvider;

        const char* endiannessMessage = endiannessProvider->IsBigEndian() ? "BigEndian\n" : "LittleEndian\n";
        printf(endiannessMessage);
    }

    void PackingSerializer::ReadPacking(string packingFilePath, Packing* particles) const
    {
        int numberOfParticles = particles->size();
        Packing& particlesRef = *particles;
        ScopedFile<ExceptionErrorHandler> file(packingFilePath, "rb");

        printf("file '%s' was opened\n", packingFilePath.c_str());
        for (int i = 0; i < numberOfParticles; ++i)
        {
            // Can not read the whole array at once, as DomainParticle objects contain more than 4 floats
            FLOAT_TYPE* data = reinterpret_cast<FLOAT_TYPE*>(&particlesRef[i]);
            ReadLittleEndian<FLOAT_TYPE>(data, 4, file);
            particlesRef[i].index = i;
        }
    }

    void PackingSerializer::ReadParticleDiameters(string diametersFilePath, Packing* particles) const
    {
        int numberOfParticles = particles->size();
        Packing& particlesRef = *particles;
        ScopedFile<LogErrorHandler> file(diametersFilePath, "r");
        if (file.Exists())
        {
            printf("Reading particle diameters from a file 'diameters'...\n");
            for (int i = 0; i < numberOfParticles; ++i)
            {
                int result = fscanf(file, "%lf", &particlesRef[i].diameter);
                bool endOfFile = result == EOF;
                bool notLastParticle = i < (numberOfParticles - 1);
                if (endOfFile && notLastParticle)
                {
                    printf("Diameters file is too short. Exiting...\n");
                    throw InvalidOperationException("Diameters file is too short.");
                }
            }
            printf("done.\n");
        }
        else
        {
            printf("Diameter of all particles is 1.0\n");
            for (int i = 0; i < numberOfParticles; ++i)
            {
                particlesRef[i].diameter = 1.0;
            }
        }
    }

    void PackingSerializer::SerializePacking(string packingFilePath, const Packing& particles) const
    {
        WritePacking(packingFilePath, particles, false);
    }

    void PackingSerializer::AppendPacking(string packingFilePath, const Packing& particles) const
    {
        WritePacking(packingFilePath, particles, true);
    }

    void PackingSerializer::WritePacking(string packingFilePath, const Packing& particles, bool shouldAppend) const
    {
        const char* mode = shouldAppend ? "ab" : "wb";
        ScopedFile<LogErrorHandler> file(packingFilePath, mode);
        int numberOfParticles = particles.size();
        for (int i = 0; i < numberOfParticles; ++i)
        {
            // Can not write the whole array at once, as DomainParticle objects contain more than 4 floats
            const FLOAT_TYPE* data = reinterpret_cast<const FLOAT_TYPE*>(&particles[i]);
            WriteLittleEndian<FLOAT_TYPE>(data, 4, file);
        }
    }

    void PackingSerializer::ReadConfig(string baseFolder, ExecutionConfig* config) const
    {
        config->generationConfig.baseFolder = baseFolder;
        printf("The current working directory is %s.\n", baseFolder.c_str());
        ScopedFile<ExceptionErrorHandler> file(Path::Append(baseFolder, CONFIG_FILE_NAME), "r");

        int boundariesMode;
        // For scanning and printing doubles, see http://stackoverflow.com/questions/210590/why-does-scanf-need-lf-for-doubles-when-printf-is-okay-with-just-f
        fscanf(file,
                "Particles count: %d\n" \
                "Packing size: " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT "\n" \
                "Generation start: %d\n" \
                "Seed: %d\n" \
                "Steps to write: %d\n" \
                "Boundaries mode: %d\n" \
                "Contraction rate: " FLOAT_FORMAT,
                &config->systemConfig.particlesCount,
                &config->systemConfig.packingSize[Axis::X],
                &config->systemConfig.packingSize[Axis::Y],
                &config->systemConfig.packingSize[Axis::Z],
                &config->generationConfig.generationStart,
                &config->generationConfig.seed,
                &config->generationConfig.stepsToWrite,
                &boundariesMode,
                &config->generationConfig.contractionRate);

        config->systemConfig.boundariesMode = static_cast<BoundariesMode::Type>(boundariesMode);

        printf("\nN is %d\ndimensions are %f %f %f\ngeneration mode: %s\nseed %d\nsteps to write intermediate state %d\nboundaries mode is %d\n\n\n",
                config->systemConfig.particlesCount,
                config->systemConfig.packingSize[Axis::X],
                config->systemConfig.packingSize[Axis::Y],
                config->systemConfig.packingSize[Axis::Z],
                config->generationConfig.generationStart == 1 ? "start" : "continue",
                config->generationConfig.seed,
                config->generationConfig.stepsToWrite,
                config->systemConfig.boundariesMode);

        if (config->systemConfig.boundariesMode == BoundariesMode::Trapezoid)
        {
            fscanf(file, "\nAlpha: " FLOAT_FORMAT, &config->systemConfig.alpha);
            if ((config->systemConfig.alpha >= 90.0) || (config->systemConfig.alpha < 0.0))
            {
                printf("Error: alpha must be non-negative and less than 90.0 degrees!\n");
                throw InvalidOperationException("Alpha is negative or less than 90.0 degrees.");
            }
            else
            {
                printf("Alpha is %lf degrees\n", config->systemConfig.alpha);
            }
        }

        int generationMode;
        int numberOfReads = fscanf(file, "\nGeneration mode: %d", &generationMode);
        config->generationConfig.initialParticleDistribution = static_cast<InitialParticleDistribution::Type>(generationMode);
        if (numberOfReads <= 0)
        {
            config->generationConfig.initialParticleDistribution = InitialParticleDistribution::Poisson;
        }
    }

    void PackingSerializer::SerializeConfig(string configFilePath, const ExecutionConfig& config) const
    {
        ScopedFile<LogErrorHandler> file(configFilePath, "w");
        fprintf(file,
                "Particles count: %d\n" \
                "Packing size: %f %f %f\n" \
                "Generation start: %d\n" \
                "Seed: %d\n" \
                "Steps to write: %d\n" \
                "Boundaries mode: %d\n" \
                "Contraction rate: %f",
                config.systemConfig.particlesCount,
                config.systemConfig.packingSize[Axis::X],
                config.systemConfig.packingSize[Axis::Y],
                config.systemConfig.packingSize[Axis::Z],
                config.generationConfig.generationStart,
                config.generationConfig.seed,
                config.generationConfig.stepsToWrite,
                config.systemConfig.boundariesMode,
                config.generationConfig.contractionRate);
    }

    void PackingSerializer::ReadPackingInfo(string infoFilePath, PackingInfo* packingInfo) const
    {
        ScopedFile<LogErrorHandler> file(infoFilePath, "r");
        FLOAT_TYPE dummyFloat;
        int dummyInt;
        fscanf(file, "N: %d\n"\
                " Dimensions: " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT "\n"\
                " Theoretical Porosity: " FLOAT_FORMAT "\n"\
                "Final Porosity: " FLOAT_FORMAT " (Tolerance: " FLOAT_FORMAT ")\n"\
                "Total Simulation Time: " FLOAT_FORMAT "\n"\
                "Total Iterations: %llu",
                &dummyInt, &dummyFloat, &dummyFloat, &dummyFloat,
                &packingInfo->theoreticalPorosity,
                &packingInfo->calculatedPorosity,
                &packingInfo->tolerance,
                &packingInfo->totalTime,
                &packingInfo->iterationsCount);

        packingInfo->runsCount = 1;
        fscanf(file, "\nRuns count: %d\n", &packingInfo->runsCount);
    }

    void PackingSerializer::SerializePackingInfo(string infoFilePath, const SystemConfig& config, const PackingInfo& packingInfo) const
    {
        ScopedFile<LogErrorHandler> file(infoFilePath, "w");
        fprintf(file, "N: %d\n"\
                " Dimensions: %f %f %f\n"\
                " Theoretical Porosity: %1.15g\n"\
                "Final Porosity: %1.15g (Tolerance: %f)\n"\
                "Total Simulation Time: %f\n"\
                "Total Iterations: %llu\n"\
                "Runs count: %d\n",
                config.particlesCount,
                config.packingSize[Axis::X],
                config.packingSize[Axis::Y],
                config.packingSize[Axis::Z],
                packingInfo.theoreticalPorosity,
                packingInfo.calculatedPorosity,
                packingInfo.tolerance,
                packingInfo.totalTime,
                packingInfo.iterationsCount,
                packingInfo.runsCount);
    }

    void PackingSerializer::SerializeInsertionRadii(string radiiFilePath, const vector<FLOAT_TYPE>& insertionRadii) const
    {
        ScopedFile<LogErrorHandler> file(radiiFilePath, "wb");
        WriteLittleEndian<FLOAT_TYPE>(insertionRadii, file);
    }

    void PackingSerializer::SerializeParticleDirections(string distancesFilePath, int particleCount, const vector<OrderService::NeighborDirections>& particleDirections) const
    {
        ScopedFile<LogErrorHandler> file(distancesFilePath, "wb");

        for (int particleIndex = 0; particleIndex < particleCount; ++particleIndex)
        {
            const vector<OrderService::NeighborDirection>& neighborDirectionsRef = particleDirections[particleIndex];
            for (vector<OrderService::NeighborDirection>::const_iterator directionsIterator = neighborDirectionsRef.begin(); directionsIterator != neighborDirectionsRef.end(); ++directionsIterator)
            {
                // Do not want a copy constructor of vector<DistanceService::DynamicSpatialVector> to be called when dereferencing the iterator, therefore assign to reference
                const OrderService::NeighborDirection& neighborDirection = *directionsIterator;
                fprintf(file, "%d %d %f %f %f\n", particleIndex, neighborDirection.neighborIndex, neighborDirection.direction[0], neighborDirection.direction[1], neighborDirection.direction[2]);
            }
        }
    }

    void PackingSerializer::SerializeContractionEnergies(string energiesFilePath,
            const vector<FLOAT_TYPE>& contractionRatios, const vector<FLOAT_TYPE>& energyPowers, const vector<FLOAT_TYPE>& contractionEnergies, const vector<int>& nonRattlersCounts) const
    {
        ScopedFile<LogErrorHandler> file(energiesFilePath, "wb");
        int contractionRatiosCount = contractionRatios.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            FLOAT_TYPE energyPerParticle = contractionEnergies[i] / nonRattlersCounts[i];
            fprintf(file, "%20.15g %20.15g %g %i %g\n", contractionRatios[i], energyPowers[i], contractionEnergies[i], nonRattlersCounts[i], energyPerParticle);
        }
    }

    void PackingSerializer::SerializeOrder(string orderFilePath, const OrderService::Order order) const
    {
        ScopedFile<LogErrorHandler> file(orderFilePath, "wb");
        fprintf(file, "Global: %f\nLocal: %f", order.globalOrder, order.localOrder);

    }

    void PackingSerializer::SerializeEntropy(string entropyFilePath, FLOAT_TYPE entropy) const
    {
        ScopedFile<LogErrorHandler> file(entropyFilePath, "wb");
        fprintf(file, "Entropy: %20.15f\n", entropy);

    }

    void PackingSerializer::SerializeLocalEntropy(string entropyFilePath, FLOAT_TYPE entropy, const vector<FLOAT_TYPE>& localEntropies) const
    {
        ScopedFile<LogErrorHandler> file(entropyFilePath, "wb");
        fprintf(file, "Entropy: %20.15f\n", entropy);
        fprintf(file, "Local entropies:\n");

        for (vector<FLOAT_TYPE>::const_iterator it = localEntropies.begin(); it != localEntropies.end(); ++it)
        {
            FLOAT_TYPE localEntropy = *it;
            fprintf(file, "%20.15f\n", localEntropy);
        }
    }

    void PackingSerializer::SerializeHessianEigenvalues(string eigenvaluesFilePath, const vector<FLOAT_TYPE>& hessianEigenvalues) const
    {
         ScopedFile<LogErrorHandler> file(eigenvaluesFilePath, "wb");
         int eigenvaluesCount = hessianEigenvalues.size();
         for (int i = 0; i < eigenvaluesCount; ++i)
         {
             fprintf(file, "%g\n", hessianEigenvalues[i]);
         }
    }

    void PackingSerializer::SerializePressures(string pressuresFilePath, const vector<FLOAT_TYPE>& contractionRatios, const vector<FLOAT_TYPE>& energyPowers, const vector<FLOAT_TYPE>& pressures) const
    {
        ScopedFile<LogErrorHandler> file(pressuresFilePath, "wb");
        int contractionRatiosCount = contractionRatios.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            fprintf(file, "%20.15g %20.15g %20.15g\n", contractionRatios[i], energyPowers[i], pressures[i]);
        }
    }

    void PackingSerializer::SerializeMolecularDynamicsStatistics(string pressuresFilePath, const vector<PressureData>& pressuresData) const
    {
        ScopedFile<LogErrorHandler> file(pressuresFilePath, "wb");
        fprintf(file, "contractionRatio density pressure jammingDensity\n");

        int contractionRatiosCount = pressuresData.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            const PressureData& pressureData = pressuresData[i];
            if (pressureData.collisionErrorsExisted)
            {
                continue;
            }

            fprintf(file, "%20.15g %20.15g %20.15g %20.15g\n",
                    pressureData.contractionRatio, pressureData.density, pressureData.pressure, pressureData.jammingDensity);
        }
    }

    void PackingSerializer::SerializePairCorrelationFunction(std::string pairCorrelationFunctionFilePath, const Model::PairCorrelationFunction& pairCorrelationFunction) const
    {
        ScopedFile<LogErrorHandler> file(pairCorrelationFunctionFilePath, "wb");
        fprintf(file, "binLeftEdge binParticleCount pairCorrelationFunctionValue\n");

        int binsCount = pairCorrelationFunction.binLeftEdges.size();
        for (int i = 0; i < binsCount; ++i)
        {
            fprintf(file, "%20.15g %d %20.15g\n",
                    pairCorrelationFunction.binLeftEdges[i], pairCorrelationFunction.binParticleCounts[i], pairCorrelationFunction.pairCorrelationFunctionValues[i]);
        }
    }

    void PackingSerializer::SerializeStructureFactor(string structureFactorFilePath, const StructureFactor& structureFactor) const
    {
        ScopedFile<LogErrorHandler> file(structureFactorFilePath, "wb");
        fprintf(file, "waveVectorLength structureFactorValue\n");

        int valuesCount = structureFactor.waveVectorLengths.size();
        for (int i = 0; i < valuesCount; ++i)
        {
            fprintf(file, "%20.15g %20.15g\n", structureFactor.waveVectorLengths[i], structureFactor.structureFactorValues[i]);
        }
    }

    void PackingSerializer::SerializeMolecularDynamicsStatistics(string statisticsFilePath, const MolecularDynamicsStatistics& statistics) const
    {
        ScopedFile<LogErrorHandler> file(statisticsFilePath, "wb");

        fprintf(file, "ExchangedMomentum: %1.15e\n", statistics.exchangedMomentum);
        fprintf(file, "KineticEnergy: %1.15e\n", statistics.kineticEnergy);
        fprintf(file, "ReducedPressure: %1.15e\n", statistics.reducedPressure);
        fprintf(file, "TimePeriod: %1.15e\n", statistics.timePeriod);
        fprintf(file, "EventsCount: %d\n", statistics.eventsCount);
        fprintf(file, "EquilibrationEventsCount: %d\n", statistics.equilibrationEventsCount);
        fprintf(file, "CollisionErrorsExisted: %d\n", statistics.collisionErrorsExisted);
    }

    void PackingSerializer::SerializeMatrix(string filePath, const FLOAT_TYPE** matrix, int dimension) const
    {
        ScopedFile<LogErrorHandler> file(filePath, "wb");

        for (int rowIndex = 0; rowIndex < dimension; ++rowIndex)
        {
            for (int columnIndex = 0; columnIndex < dimension; ++columnIndex)
            {
                fprintf(file, "%f ", matrix[rowIndex][columnIndex]);
            }
            fprintf(file, "\n");
        }
    }

    void PackingSerializer::ReadInsertionRadii(string radiiFilePath, vector<FLOAT_TYPE>* insertionRadii) const
    {
        ScopedFile<LogErrorHandler> file(radiiFilePath, "r");
        vector<FLOAT_TYPE>& insertionRadiiRef = *insertionRadii;
        ReadLittleEndian<FLOAT_TYPE>(&insertionRadiiRef[0], insertionRadii->size(), file);
    }

    void PackingSerializer::ReadParticleDistances(string distancesFilePath, int particleCount, FLOAT_TYPE** particleDistances) const
    {
        ScopedFile<LogErrorHandler> file(distancesFilePath, "r");
        for (int i = 0; i < particleCount; ++i)
        {
            ReadLittleEndian<FLOAT_TYPE>(particleDistances[i], particleCount, file);
        }
    }

    PackingSerializer::~PackingSerializer()
    {

    }
}

