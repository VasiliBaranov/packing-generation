// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingSerializer.h"

#include <cmath>
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
    // TODO: refactor, extract vector serialization
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
        ScopedFile<ExceptionErrorHandler> file(packingFilePath, FileOpenMode::Read | FileOpenMode::Binary);

        printf("file '%s' was opened\n", packingFilePath.c_str());
        for (int i = 0; i < numberOfParticles; ++i)
        {
            // Can not read the whole array at once, as DomainParticle objects contain more than 4 floats
            FLOAT_TYPE* data = reinterpret_cast<FLOAT_TYPE*>(&particlesRef[i]);
            const int floatsPerParticle = DIMENSIONS + 1;
            bool success = ReadLittleEndian<FLOAT_TYPE>(data, floatsPerParticle, file);
            if (!success)
            {
                throw InvalidOperationException("Particles' file is too small.");
            }
            particlesRef[i].index = i;
        }
    }

    void PackingSerializer::ReadParticleDiameters(string diametersFilePath, Packing* particles) const
    {
        int numberOfParticles = particles->size();
        Packing& particlesRef = *particles;
        ScopedFile<LogErrorHandler> file(diametersFilePath, FileOpenMode::Read);
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

    void PackingSerializer::ReadImmobileParticleIndexes(string immobileParticlesPath, vector<ParticleIndex>* immobileParticleIndexesPtr) const
    {
        vector<ParticleIndex>& immobileParticleIndexes = *immobileParticleIndexesPtr;
        immobileParticleIndexes.clear();

        ScopedFile<LogErrorHandler> file(immobileParticlesPath, FileOpenMode::Read);
        while (true)
        {
            ParticleIndex immobileIndex;
            int result = fscanf(file, "%d\n", &immobileIndex);
            bool endOfFile = result == EOF;
            if (endOfFile)
            {
                break;
            }

            immobileParticleIndexes.push_back(immobileIndex);
        }
    }

    void PackingSerializer::ReadContractionEnergies(string energiesFilePath, vector<FLOAT_TYPE>* contractionRatios,
            vector<FLOAT_TYPE>* energyPowers, vector<FLOAT_TYPE>* contractionEnergies, vector<int>* nonRattlersCounts) const
    {
        ScopedFile<ExceptionErrorHandler> file(energiesFilePath, FileOpenMode::Read);
        // TODO: extract into ReadTable(string filePath, vector<vector<string> >* table) and into ReadTable<T>(string filePath, vector<vector<T> >* table)
        while(true)
        {
            FLOAT_TYPE contractionRatio;
            FLOAT_TYPE energyPower;
            FLOAT_TYPE contractionEnergy;
            int nonRattlersCount;
            FLOAT_TYPE energyPerParticle;
            int result = fscanf(file, "%lg %lg %lg %i %lg\n", &contractionRatio, &energyPower, &contractionEnergy, &nonRattlersCount, &energyPerParticle);

            bool endOfFile = result == EOF;
            if (endOfFile)
            {
                return;
            }

            contractionRatios->push_back(contractionRatio);
            energyPowers->push_back(energyPower);
            contractionEnergies->push_back(contractionEnergy);
            nonRattlersCounts->push_back(nonRattlersCount);
        }
    }

    void PackingSerializer::ReadNearestNeighbors(string neighborsFilePath, vector<ParticlePair>* closestPairs) const
    {
        closestPairs->clear();
        ScopedFile<ExceptionErrorHandler> file(neighborsFilePath, FileOpenMode::Read);
        fscanf(file, "firstParticleIndex secondParticleIndex normalizedClosestDistance isFirstParticleImmobile\n"); // TODO: skip the first line correctly
        while(true)
        {
            ParticleIndex firstParticleIndex;
            ParticleIndex secondParticleIndex;
            FLOAT_TYPE normalizedClosestDistance;
            int isImmobile;
            int result = fscanf(file, "%i %i %lg %i\n", &firstParticleIndex, &secondParticleIndex, &normalizedClosestDistance, &isImmobile);

            bool endOfFile = result < 3;
            if (endOfFile)
            {
                return;
            }

            ParticlePair pair(firstParticleIndex, secondParticleIndex, normalizedClosestDistance);

            closestPairs->push_back(pair);
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
        FileOpenMode::Type mode = shouldAppend ? (FileOpenMode::Append | FileOpenMode::Binary) : (FileOpenMode::Write | FileOpenMode::Binary);
        ScopedFile<LogErrorHandler> file(packingFilePath, mode);
        int numberOfParticles = particles.size();
        for (int i = 0; i < numberOfParticles; ++i)
        {
            // Can not write the whole array at once, as DomainParticle objects contain more than 4 floats
            const FLOAT_TYPE* data = reinterpret_cast<const FLOAT_TYPE*>(&particles[i]);
            const int floatsPerParticle = DIMENSIONS + 1;
            WriteLittleEndian<FLOAT_TYPE>(data, floatsPerParticle, file);
        }
    }

    void PackingSerializer::ReadConfig(string baseFolder, ExecutionConfig* config) const
    {
        config->generationConfig.baseFolder = baseFolder;
        printf("The current working directory is %s.\n", baseFolder.c_str());
        ScopedFile<ExceptionErrorHandler> file(Path::Append(baseFolder, CONFIG_FILE_NAME), FileOpenMode::Read);
        boost::array<FLOAT_TYPE, 3> systemSize; // need a separate 3D vector, as the system may be two-dimensional.

        int boundariesMode;
        int shouldStartGeneration;
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
                &systemSize[Axis::X],
                &systemSize[Axis::Y],
                &systemSize[Axis::Z],
                &shouldStartGeneration,
                &config->generationConfig.seed,
                &config->generationConfig.stepsToWrite,
                &boundariesMode,
                &config->generationConfig.contractionRate);

        for (int i = 0; i < DIMENSIONS; ++i)
        {
            config->systemConfig.packingSize[i] = systemSize[i];
        }

        config->systemConfig.boundariesMode = static_cast<BoundariesMode::Type>(boundariesMode);
        config->generationConfig.shouldStartGeneration.value = (shouldStartGeneration > 0);
        config->generationConfig.shouldStartGeneration.hasValue = true;

        bool unknownBoundariesMode = (boundariesMode != BoundariesMode::Bulk) &&
                (boundariesMode != BoundariesMode::Ellipse) &&
                (boundariesMode != BoundariesMode::Rectangle) &&
                (boundariesMode != BoundariesMode::Trapezoid);

        if (unknownBoundariesMode)
        {
            printf("Right now boundaries modes 1 (periodic XYZ), 2 (periodic Z, circle XY (Y is diameter) ), 3 (periodic Z, rectangle XY), or 4 (periodic Z, trapezoid XY) are supported  only\n");
            throw InvalidOperationException("Incorrect boundary mode.");
        }

        printf("\nN is %d\ndimensions are %f %f %f\ngeneration mode: %s\nseed %d\nsteps to write intermediate state %d\nboundaries mode is %d\n\n\n",
                config->systemConfig.particlesCount,
                systemSize[Axis::X],
                systemSize[Axis::Y],
                systemSize[Axis::Z],
                (config->generationConfig.shouldStartGeneration.hasValue && config->generationConfig.shouldStartGeneration.value) ? "start" : "continue",
                config->generationConfig.seed,
                config->generationConfig.stepsToWrite,
                config->systemConfig.boundariesMode);

        // Optional parameters

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

        FLOAT_TYPE finalContractionRate;
        numberOfReads = fscanf(file, "\nFinal contraction rate: " FLOAT_FORMAT, &finalContractionRate);
        if (numberOfReads > 0)
        {
            config->generationConfig.finalContractionRate = finalContractionRate;
        }
        else
        {
            config->generationConfig.finalContractionRate = 1e-4;
        }

        FLOAT_TYPE contractionRateDecreaseFactor;
        numberOfReads = fscanf(file, "\nContraction rate decrease factor: " FLOAT_FORMAT, &contractionRateDecreaseFactor);
        if (numberOfReads > 0)
        {
            config->generationConfig.contractionRateDecreaseFactor = contractionRateDecreaseFactor;
        }
        else
        {
            config->generationConfig.contractionRateDecreaseFactor = 2.0;
        }
    }

    void PackingSerializer::ReadActiveConfig(std::string activeConfigPath, Model::SystemConfig* activeConfig, Core::SpatialVector* shift) const
    {
        ScopedFile<ExceptionErrorHandler> file(activeConfigPath, FileOpenMode::Read);
        boost::array<FLOAT_TYPE, 3> systemSize; // need a separate 3D vector, as the system may be two-dimensional.
        boost::array<FLOAT_TYPE, 3> shiftLocal;

        int boundariesMode;
        // For scanning and printing doubles, see http://stackoverflow.com/questions/210590/why-does-scanf-need-lf-for-doubles-when-printf-is-okay-with-just-f
        fscanf(file,
                "Active area size: " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT "\n" \
                "Active area shift: " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT "\n" \
                "Boundaries mode: %d\n" \
                "Active particles count: %d\n",
                &systemSize[Axis::X],
                &systemSize[Axis::Y],
                &systemSize[Axis::Z],
                &shiftLocal[Axis::X],
                &shiftLocal[Axis::Y],
                &shiftLocal[Axis::Z],
                &boundariesMode,
                &activeConfig->particlesCount);

        SpatialVector& shiftRef = *shift;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            activeConfig->packingSize[i] = systemSize[i];
            shiftRef[i] = shiftLocal[i];
        }

        activeConfig->boundariesMode = static_cast<BoundariesMode::Type>(boundariesMode);

        bool unknownBoundariesMode = (boundariesMode != BoundariesMode::Bulk) &&
                (boundariesMode != BoundariesMode::Ellipse) &&
                (boundariesMode != BoundariesMode::Rectangle) &&
                (boundariesMode != BoundariesMode::Trapezoid);

        if (unknownBoundariesMode)
        {
            printf("Right now boundaries modes 1 (periodic XYZ), 2 (periodic Z, circle XY (Y is diameter) ), 3 (periodic Z, rectangle XY), or 4 (periodic Z, trapezoid XY) are supported  only\n");
            throw InvalidOperationException("Incorrect boundary mode.");
        }
    }

    void PackingSerializer::SerializeConfig(string configFilePath, const ExecutionConfig& config) const
    {
        boost::array<FLOAT_TYPE, 3> systemSize = MakeSpatialVectorThreeDimensional(config.systemConfig.packingSize);

        int shouldStartGeneration;
        if (config.generationConfig.shouldStartGeneration.hasValue)
        {
            shouldStartGeneration = config.generationConfig.shouldStartGeneration.value ? 1 : 0;
        }
        else
        {
            shouldStartGeneration = 0;
        }
        ScopedFile<LogErrorHandler> file(configFilePath, FileOpenMode::Write);
        fprintf(file,
                "Particles count: %d\n" \
                "Packing size: %.13f %.13f %.13f\n" \
                "Generation start: %d\n" \
                "Seed: %d\n" \
                "Steps to write: %d\n" \
                "Boundaries mode: %d\n" \
                "Contraction rate: %g\n",
                config.systemConfig.particlesCount,
                systemSize[Axis::X],
                systemSize[Axis::Y],
                systemSize[Axis::Z],
                shouldStartGeneration,
                config.generationConfig.seed,
                config.generationConfig.stepsToWrite,
                config.systemConfig.boundariesMode,
                config.generationConfig.contractionRate);

        if (config.generationConfig.finalContractionRate > 0)
        {
            fprintf(file, "Final contraction rate: %f\n", config.generationConfig.finalContractionRate);
        }

        if (config.generationConfig.contractionRateDecreaseFactor > 0)
        {
            fprintf(file, "Contraction rate decrease factor: %f\n", config.generationConfig.contractionRateDecreaseFactor);
        }

        fprintf(file,
        "1. boundaries mode: 1 - bulk; 2 - ellipse (inscribed in XYZ box, Z is length of an ellipse); 3 - rectangle\n" \
        "2. generationMode = 1 (Poisson, R) or 2 (Poisson in cells, S)");
    }

    void PackingSerializer::SerializeActiveConfig(string activeConfigPath, const SystemConfig& activeConfig, const SpatialVector& shift) const
    {
        boost::array<FLOAT_TYPE, 3> systemSize = MakeSpatialVectorThreeDimensional(activeConfig.packingSize);
        boost::array<FLOAT_TYPE, 3> shiftLocal = MakeSpatialVectorThreeDimensional(shift);

        ScopedFile<LogErrorHandler> file(activeConfigPath, FileOpenMode::Write);
        fprintf(file,
                "Active area size: %.13f %.13f %.13f\n" \
                "Active area shift: %.13f %.13f %.13f\n" \
                "Boundaries mode: %d\n" \
                "Active particles count: %d\n",
                systemSize[Axis::X],
                systemSize[Axis::Y],
                systemSize[Axis::Z],
                shiftLocal[Axis::X],
                shiftLocal[Axis::Y],
                shiftLocal[Axis::Z],
                activeConfig.boundariesMode,
                activeConfig.particlesCount);

        fprintf(file, "1. boundaries mode: 1 - bulk; 2 - ellipse (inscribed in XYZ box, Z is length of an ellipse); 3 - rectangle\n");
    }

    void PackingSerializer::ReadPackingInfo(string infoFilePath, PackingInfo* packingInfo) const
    {
        ScopedFile<LogErrorHandler> file(infoFilePath, FileOpenMode::Read);
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
    }

    void PackingSerializer::SerializePackingInfo(string infoFilePath, const SystemConfig& config, const PackingInfo& packingInfo) const
    {
        boost::array<FLOAT_TYPE, 3> systemSize = MakeSpatialVectorThreeDimensional(config.packingSize);

        ScopedFile<LogErrorHandler> file(infoFilePath, FileOpenMode::Write);
        fprintf(file, "N: %d\n"\
                " Dimensions: %f %f %f\n"\
                " Theoretical Porosity: %1.15g\n"\
                "Final Porosity: %1.15g (Tolerance: %f)\n"\
                "Total Simulation Time: %f\n"\
                "Total Iterations: %llu\n",
                config.particlesCount,
                systemSize[Axis::X],
                systemSize[Axis::Y],
                systemSize[Axis::Z],
                packingInfo.theoreticalPorosity,
                packingInfo.calculatedPorosity,
                packingInfo.tolerance,
                packingInfo.totalTime,
                packingInfo.iterationsCount);
    }

    void PackingSerializer::SerializeInsertionRadii(string radiiFilePath, const vector<FLOAT_TYPE>& insertionRadii) const
    {
        ScopedFile<LogErrorHandler> file(radiiFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        WriteLittleEndian<FLOAT_TYPE>(insertionRadii, file);
    }

    void PackingSerializer::SerializeDistancesToSurfaces(string distancesFolderPath, const vector<int>& surfaceIndexes, const vector<vector<FLOAT_TYPE> >& distancesToSurfaces, bool shouldAppend) const
    {
        if (!shouldAppend)
        {
            Path::EnsureClearDirectory(distancesFolderPath);
        }
        FileOpenMode::Type mode = shouldAppend ? (FileOpenMode::Append | FileOpenMode::Binary) : (FileOpenMode::Write | FileOpenMode::Binary);

        vector<FLOAT_TYPE> distancesToCurrentSurfaces(distancesToSurfaces.size());

        for (size_t surfaceIndex = 0; surfaceIndex < surfaceIndexes.size(); ++surfaceIndex)
        {
            for (size_t samplePointIndex = 0; samplePointIndex < distancesToSurfaces.size(); ++samplePointIndex)
            {
                distancesToCurrentSurfaces[samplePointIndex] = distancesToSurfaces[samplePointIndex][surfaceIndex];
            }

            string distancesToSurfacesFileName = "surface_index_" + Utilities::ConvertToString(surfaceIndexes[surfaceIndex]) + ".bin";
            string distancesToSurfacesPath = Path::Append(distancesFolderPath, distancesToSurfacesFileName);
            ScopedFile<LogErrorHandler> distancesFile(distancesToSurfacesPath, mode);
            WriteLittleEndian<FLOAT_TYPE>(distancesToCurrentSurfaces, distancesFile);
        }
    }

    void PackingSerializer::SerializeContactNumberDistribution(string contactNumberDistributionFilePath, const vector<int>& neighborCounts, const vector<int>& neighborCountFrequencies) const
    {
        ScopedFile<LogErrorHandler> file(contactNumberDistributionFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "neighborCounts neighborCountFrequencies\n");

        for (size_t i = 0; i < neighborCounts.size(); ++i)
        {
            fprintf(file, "%d %d\n", neighborCounts[i], neighborCountFrequencies[i]);
        }
    }

    void PackingSerializer::SerializeContactingNeighborIndexes(string contactingNeighborIndexesFilePath, const vector<vector<int>>& neighborIndexes) const
    {
        ScopedFile<LogErrorHandler> file(contactingNeighborIndexesFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "contactingNeighborIndexesPerParticle\n");

        // TODO: make a helper function to store lists of lists
        for (int particleIndex = 0; particleIndex < neighborIndexes.size(); particleIndex++)
        {
            const std::vector<int>& currentNeighborIndexes = neighborIndexes[particleIndex];
            if (currentNeighborIndexes.size() > 0)
            {
                for (int i = 0; i < currentNeighborIndexes.size() - 1; i++)
                {
                    fprintf(file, "%d ", currentNeighborIndexes[i]);
                }
                fprintf(file, "%d", currentNeighborIndexes[currentNeighborIndexes.size() - 1]);
            }
            if (particleIndex < (neighborIndexes.size() - 1))
            {
                fprintf(file, "\n");
            }
        }
    }

    void PackingSerializer::SerializeContactingNeighborDistances(string contactingNeighborDistancesFilePath, const vector<FLOAT_TYPE>& contactingNeighborDistances) const
    {
        ScopedFile<LogErrorHandler> file(contactingNeighborDistancesFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "contactingNeighborDistances\n");

        for (size_t i = 0; i < contactingNeighborDistances.size(); ++i)
        {
            fprintf(file, "%f\n", contactingNeighborDistances[i]);
        }
    }

    void PackingSerializer::SerializeParticleDirections(string distancesFilePath, int particleCount, const vector<OrderService::NeighborDirections>& particleDirections) const
    {
        ScopedFile<LogErrorHandler> file(distancesFilePath, FileOpenMode::Write | FileOpenMode::Binary);

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
        ScopedFile<LogErrorHandler> file(energiesFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        int contractionRatiosCount = contractionRatios.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            FLOAT_TYPE energyPerParticle = contractionEnergies[i] / nonRattlersCounts[i];
            fprintf(file, "%20.15g %20.15g %g %i %g\n", contractionRatios[i], energyPowers[i], contractionEnergies[i], nonRattlersCounts[i], energyPerParticle);
        }
    }

    void PackingSerializer::SerializeOrder(string orderFilePath, const OrderService::Order& order) const
    {
        ScopedFile<LogErrorHandler> file(orderFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "Global: %f\nLocal: %f", order.globalOrder, order.localOrder);

    }

    void PackingSerializer::SerializeEntropy(string entropyFilePath, FLOAT_TYPE entropy) const
    {
        ScopedFile<LogErrorHandler> file(entropyFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "Entropy: %20.15f\n", entropy);

    }

    void PackingSerializer::SerializeLocalEntropy(string entropyFilePath, FLOAT_TYPE entropy, const vector<FLOAT_TYPE>& localEntropies) const
    {
        ScopedFile<LogErrorHandler> file(entropyFilePath, FileOpenMode::Write | FileOpenMode::Binary);
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
         ScopedFile<LogErrorHandler> file(eigenvaluesFilePath, FileOpenMode::Write | FileOpenMode::Binary);
         int eigenvaluesCount = hessianEigenvalues.size();
         for (int i = 0; i < eigenvaluesCount; ++i)
         {
             fprintf(file, "%g\n", hessianEigenvalues[i]);
         }
    }

    void PackingSerializer::SerializePressures(string pressuresFilePath, const vector<FLOAT_TYPE>& contractionRatios, const vector<FLOAT_TYPE>& energyPowers, const vector<FLOAT_TYPE>& pressures) const
    {
        ScopedFile<LogErrorHandler> file(pressuresFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        int contractionRatiosCount = contractionRatios.size();
        for (int i = 0; i < contractionRatiosCount; ++i)
        {
            fprintf(file, "%20.15g %20.15g %20.15g\n", contractionRatios[i], energyPowers[i], pressures[i]);
        }
    }

    void PackingSerializer::SerializeMolecularDynamicsStatistics(string pressuresFilePath, const vector<PressureData>& pressuresData) const
    {
        ScopedFile<LogErrorHandler> file(pressuresFilePath, FileOpenMode::Write | FileOpenMode::Binary);
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
        ScopedFile<LogErrorHandler> file(pairCorrelationFunctionFilePath, FileOpenMode::Write | FileOpenMode::Binary);
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
        ScopedFile<LogErrorHandler> file(structureFactorFilePath, FileOpenMode::Write | FileOpenMode::Binary);
        fprintf(file, "waveVectorLength structureFactorValue\n");

        int valuesCount = structureFactor.waveVectorLengths.size();
        for (int i = 0; i < valuesCount; ++i)
        {
            fprintf(file, "%20.15g %20.15g\n", structureFactor.waveVectorLengths[i], structureFactor.structureFactorValues[i]);
        }
    }

    void PackingSerializer::SerializeLocalOrientationalDisorder(string disorderFilePath, const OrderService::LocalOrientationalDisorder& disorder) const
    {
        ScopedFile<LogErrorHandler> file(disorderFilePath, FileOpenMode::Write);

        // TODO: add a method to print vectors
        size_t length = disorder.referenceLatticeNames.size();
        for (size_t i = 0; i < length - 1; ++i)
        {
            fprintf(file, "%s ", disorder.referenceLatticeNames[i].c_str());
        }
        fprintf(file, "%s\n", disorder.referenceLatticeNames[length - 1].c_str());

        for (size_t i = 0; i < disorder.disordersPerParticle.size(); ++i)
        {
            const vector<FLOAT_TYPE>& currentDisorders = disorder.disordersPerParticle[i];
            size_t length = currentDisorders.size();
            for (size_t j = 0; j < length - 1; ++j)
            {
                fprintf(file, "%g ", currentDisorders[j]);
            }
            fprintf(file, "%g\n", currentDisorders[length - 1]);
        }
    }

    void PackingSerializer::SerializeCloseNeighbors(std::string neighborsFilePath, const OrderService::LocalOrientationalDisorder& disorder) const
    {
        ScopedFile<LogErrorHandler> file(neighborsFilePath, FileOpenMode::Write);

        for (size_t i = 0; i < disorder.closeNeighborsPerParticle.size(); ++i)
        {
            const vector<int>& neighborIndexes = disorder.closeNeighborsPerParticle[i];
            size_t length = neighborIndexes.size();
            for (size_t j = 0; j < length - 1; ++j)
            {
                fprintf(file, "%d ", neighborIndexes[j]);
            }
            fprintf(file, "%d\n", neighborIndexes[length - 1]);
        }
    }

    void PackingSerializer::SerializeNeighborVectorSums(std::string neighborVectorSumsFilePath, const std::vector<Core::FLOAT_TYPE>& neighborVectorSumsNorms, const std::vector<bool>& rattlerMask, Core::FLOAT_TYPE maxNeighborVectorSumForNonRattlers) const
    {
        ScopedFile<LogErrorHandler> file(neighborVectorSumsFilePath, FileOpenMode::Write | FileOpenMode::Binary);

        fprintf(file, "Max neighbor vector sum for non-rattlers : %g\n", maxNeighborVectorSumForNonRattlers);
        fprintf(file, "NeighborVectorSumsNorm IsRattler\n");

        for (size_t i = 0; i < neighborVectorSumsNorms.size(); ++i)
        {
            fprintf(file, "%g %i\n", neighborVectorSumsNorms[i], rattlerMask[i]);
        }
    }

    void PackingSerializer::SerializeNearestNeighbors(string nearestNeighborsFilePath, const vector<ParticlePair>& closestPairs, const vector<bool>& isImmobileMask) const
    {
        ScopedFile<LogErrorHandler> file(nearestNeighborsFilePath, FileOpenMode::Write | FileOpenMode::Binary);

        fprintf(file, "firstParticleIndex secondParticleIndex normalizedClosestDistance isFirstParticleImmobile\n");

        for (size_t i = 0; i < closestPairs.size(); ++i)
        {
            fprintf(file, "%i %i %g %i\n", closestPairs[i].firstParticleIndex, closestPairs[i].secondParticleIndex, sqrt(closestPairs[i].normalizedDistanceSquare), isImmobileMask[i] ? 1 : 0);
        }
    }

    void PackingSerializer::SerializeMolecularDynamicsStatistics(string statisticsFilePath, const MolecularDynamicsStatistics& statistics) const
    {
        ScopedFile<LogErrorHandler> file(statisticsFilePath, FileOpenMode::Write | FileOpenMode::Binary);

        fprintf(file, "ExchangedMomentum: %1.15e\n", statistics.exchangedMomentum);
        fprintf(file, "KineticEnergy: %1.15e\n", statistics.kineticEnergy);
        fprintf(file, "ReducedPressure: %1.15e\n", statistics.reducedPressure);
        fprintf(file, "TimePeriod: %1.15e\n", statistics.timePeriod);
        fprintf(file, "EventsCount: %d\n", statistics.eventsCount);
        fprintf(file, "EquilibrationEventsCount: %llu\n", statistics.equilibrationEventsCount);
        fprintf(file, "CollisionErrorsExisted: %d\n", statistics.collisionErrorsExisted);
    }

    void PackingSerializer::SerializeMatrix(string filePath, const FLOAT_TYPE** matrix, int dimension) const
    {
        ScopedFile<LogErrorHandler> file(filePath, FileOpenMode::Write | FileOpenMode::Binary);

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
        ScopedFile<LogErrorHandler> file(radiiFilePath, FileOpenMode::Read);
        vector<FLOAT_TYPE>& insertionRadiiRef = *insertionRadii;
        ReadLittleEndian<FLOAT_TYPE>(&insertionRadiiRef[0], insertionRadii->size(), file);
    }

    void PackingSerializer::ReadParticleDistances(string distancesFilePath, int particleCount, FLOAT_TYPE** particleDistances) const
    {
        ScopedFile<LogErrorHandler> file(distancesFilePath, FileOpenMode::Read);
        for (int i = 0; i < particleCount; ++i)
        {
            ReadLittleEndian<FLOAT_TYPE>(particleDistances[i], particleCount, file);
        }
    }

    boost::array<FLOAT_TYPE, 3> PackingSerializer::MakeSpatialVectorThreeDimensional(const Core::SpatialVector& vector) const
    {
        boost::array<FLOAT_TYPE, 3> systemSize;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            systemSize[i] = vector[i];
        }
        return systemSize;
    }

    PackingSerializer::~PackingSerializer()
    {

    }
}

