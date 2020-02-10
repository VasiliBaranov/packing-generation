// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Model_Headers_Config_h
#define Generation_Model_Headers_Config_h

#include "Types.h"
#include "Core/Headers/Macros.h"
namespace Geometries { class IGeometry; }

namespace Model
{
    template<class TConfig>
    class IConfig
    {
    public:
        virtual void MergeWith(const TConfig& config) = 0;

        virtual void Reset() = 0;

        virtual ~IConfig(){ };
    };

    // Represents a system config (parameters, independent of algorithm).
    // NOTE: If geometry params become too diverse, introduce GeometryConfig (and descendants),
    // include it in SystemConfig, but not in ModellingContext.
    class SystemConfig : public virtual IConfig<SystemConfig>
    {
    public:
        ParticleIndex particlesCount;

        // Geometry params
        Core::SpatialVector packingSize;
        BoundariesMode::Type boundariesMode;
        double alpha; // relates just to trapezoid geometry; bullshit, but for now it's ok

    public:
        SystemConfig();

        OVERRIDE void Reset();

        OVERRIDE void MergeWith(const SystemConfig& config);

    private:
        DISALLOW_COPY_AND_ASSIGN(SystemConfig);
    };

    // Represents a generation config.
    // Currently it combines parameters needed for all algorithms (packing generation, insertion radii generation, etc).
    // NOTE: If algorithms become too diverse, introduce AlgorithmConfig (and descendants, one of them is GenerationConfig).
    class GenerationConfig : public virtual IConfig<GenerationConfig>
    {
    public:
        std::string baseFolder;
        ExecutionMode::Type executionMode;

        // Params for packing generation
        Core::Nullable<bool> shouldStartGeneration;
        int seed;
        int stepsToWrite;
        InitialParticleDistribution::Type initialParticleDistribution;
        Core::FLOAT_TYPE contractionRate;
        Core::FLOAT_TYPE finalContractionRate; // only for LubachevskyStillingerGradualDensification
        Core::FLOAT_TYPE contractionRateDecreaseFactor; // only for LubachevskyStillingerGradualDensification
        Core::Nullable<bool> shouldSuppressCrystallization;

        // Params for packing generation, usually set manually
        PackingGenerationAlgorithm::Type generationAlgorithm;

        // Params for other algorithms
        int insertionRadiiCount;
        int particlesToKeepForStructureFactor;
        Core::Nullable<bool> keepSmallParticlesForStructureFactor;

    public:
        GenerationConfig();

        OVERRIDE void Reset();

        OVERRIDE void MergeWith(const GenerationConfig& config);

    private:
        DISALLOW_COPY_AND_ASSIGN(GenerationConfig);
    };

    // Represents an Execution config, i.e. includes system config (parameters, independent of algorithm) and algorithm config.
    class ExecutionConfig : public virtual IConfig<ExecutionConfig>
    {
    public:
        SystemConfig systemConfig;
        GenerationConfig generationConfig;

    public:
        ExecutionConfig();

        virtual ~ExecutionConfig();

        OVERRIDE void Reset();

        OVERRIDE void MergeWith(const ExecutionConfig& config);

    private:
        DISALLOW_COPY_AND_ASSIGN(ExecutionConfig);
    };

    // Contains all the parameters of the modelled system and all the objects, describing the system
    // (geometry; local geometry, global geometry for parallel applications; equilibrium velocity provider, etc).

    // Do not put System (aka DomainParticle[]) here, as we separate modelling context and modelled system (particles).
    // Sometimes it requires extra Init call for services, but several services do not require system, but require context.
    class ModellingContext
    {
    public:
        SystemConfig* config;
        Geometries::IGeometry* geometry;
        Geometries::IGeometry* activeGeometry;

    public:
        ModellingContext(SystemConfig* config, Geometries::IGeometry* geometry, Geometries::IGeometry* activeGeometry = NULL);

    private:
        DISALLOW_COPY_AND_ASSIGN(ModellingContext);
    };
}

#endif /* Generation_Model_Headers_Config_h */

