// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Config.h"
#include <string>
#include "Generation/Geometries/Headers/IGeometry.h"

using namespace Core;
using namespace Geometries;

namespace Model
{
    SystemConfig::SystemConfig()
    {
        Reset();
    }

    void SystemConfig::Reset()
    {
        particlesCount = -1;
        packingSize[0] = -1;
        alpha = -1;
        boundariesMode = BoundariesMode::Unknown;
    }

    void SystemConfig::MergeWith(const SystemConfig& config)
    {
        if (particlesCount < 0)
        {
            particlesCount = config.particlesCount;
        }

        if (packingSize[0] < 0)
        {
            packingSize = config.packingSize;
        }

        if (boundariesMode == BoundariesMode::Unknown)
        {
            boundariesMode = config.boundariesMode;
        }

        if (alpha < 0)
        {
            alpha = config.alpha;
        }
    }

    GenerationConfig::GenerationConfig()
    {
        Reset();
    }

    void GenerationConfig::Reset()
    {
        shouldStartGeneration.hasValue = false;
        shouldStartGeneration.value = false;
        seed = -1;
        stepsToWrite = -1;
        baseFolder = "";
        executionMode = ExecutionMode::Unknown;
        initialParticleDistribution = InitialParticleDistribution::Unknown;

        shouldSuppressCrystallization.hasValue = false;
        shouldSuppressCrystallization.value = false;

        contractionRate = -1;
        finalContractionRate = -1;
        contractionRateDecreaseFactor = -1;
        generationAlgorithm = PackingGenerationAlgorithm::Unknown;

        insertionRadiiCount = -1;
        particlesToKeepForStructureFactor = -1;
        keepSmallParticlesForStructureFactor.hasValue = false;
    }

    void GenerationConfig::MergeWith(const GenerationConfig& config)
    {
        if (!shouldStartGeneration.hasValue)
        {
            shouldStartGeneration = config.shouldStartGeneration;
        }

        if (!shouldSuppressCrystallization.hasValue)
        {
            shouldSuppressCrystallization = config.shouldSuppressCrystallization;
        }

        if (seed < 0)
        {
            seed = config.seed;
        }

        if (stepsToWrite < 0)
        {
            stepsToWrite = config.stepsToWrite;
        }

        if (baseFolder == "")
        {
            baseFolder = config.baseFolder;
        }

        if (executionMode == ExecutionMode::Unknown)
        {
            executionMode = config.executionMode;
        }

        if (initialParticleDistribution == InitialParticleDistribution::Unknown)
        {
            initialParticleDistribution = config.initialParticleDistribution;
        }

        if (generationAlgorithm == PackingGenerationAlgorithm::Unknown)
        {
            generationAlgorithm = config.generationAlgorithm;
        }

        if (contractionRate < 0)
        {
            contractionRate = config.contractionRate;
        }

        if (finalContractionRate < 0)
        {
            finalContractionRate = config.finalContractionRate;
        }

        if (contractionRateDecreaseFactor < 0)
        {
            contractionRateDecreaseFactor = config.contractionRateDecreaseFactor;
        }

        if (insertionRadiiCount < 0)
        {
            insertionRadiiCount = config.insertionRadiiCount;
        }
        if (particlesToKeepForStructureFactor < 0)
        {
            particlesToKeepForStructureFactor = config.particlesToKeepForStructureFactor;
        }
        if (!keepSmallParticlesForStructureFactor.hasValue)
        {
            keepSmallParticlesForStructureFactor = config.keepSmallParticlesForStructureFactor;
        }
    }

    ExecutionConfig::ExecutionConfig()
    {
    }

    ExecutionConfig::~ExecutionConfig()
    {

    }
    void ExecutionConfig::Reset()
    {
        systemConfig.Reset();
        generationConfig.Reset();
    }

    void ExecutionConfig::MergeWith(const ExecutionConfig& config)
    {
        systemConfig.MergeWith(config.systemConfig);
        generationConfig.MergeWith(config.generationConfig);
    }

    ModellingContext::ModellingContext(SystemConfig* config, IGeometry* geometry, IGeometry* activeGeometry)
    {
        this->config = config;
        this->geometry = geometry;
        this->activeGeometry = activeGeometry;
    }
}

