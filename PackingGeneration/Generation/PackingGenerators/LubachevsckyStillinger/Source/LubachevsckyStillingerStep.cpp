// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/LubachevsckyStillingerStep.h"

#include <stdio.h>

// Event providers
#include "../Headers/VoronoiTesselationProvider.h"
#include "../Headers/CollisionEventProvider.h"
#include "../Headers/WallTransferEventProvider.h"
#include "../Headers/NeighborTransferEventProvider.h"
#include "../Headers/VoronoiTransferEventProvider.h"
#include "../Headers/CompositeEventProvider.h"

// Event processors
#include "../Headers/CollisionEventProcessor.h"
#include "../Headers/WallTransferEventProcessor.h"
#include "../Headers/NeighborTransferEventProcessor.h"
#include "../Headers/VoronoiTransferEventProcessor.h"
#include "../Headers/CompositeEventProcessor.h"
#include "../Headers/MoveEventProcessor.h"

#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/MemoryUtility.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/ScopedFile.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/MpiManager.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/PackingServices/DistanceServices/Headers/IClosestPairProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/Constants.h"

#include "Generation/PackingServices/EnergyServices/Headers/HarmonicPotential.h"
#include "Generation/PackingServices/EnergyServices/Headers/IEnergyService.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    const FLOAT_TYPE LubachevsckyStillingerStep::maxPressure = 1e12;

    LubachevsckyStillingerStep::LubachevsckyStillingerStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            IClosestPairProvider* distanceService,
            MathService* mathService,
            PackingSerializer* packingSerializer,
            IEnergyService* contractionEnergyService) :
            BasePackingStep(geometryService, neighborProvider, distanceService, mathService),
            particleCollisionService(mathService)
    {
        this->contractionEnergyService = contractionEnergyService;
        eventsPerParticle = 20;

        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = true;

        lockParticles = false;
        preserveInitialDiameter = false;
    }

    LubachevsckyStillingerStep::~LubachevsckyStillingerStep()
    {
    }

    void LubachevsckyStillingerStep::SetParticles(Packing* particles)
    {
        BasePackingStep::SetParticles(particles);
        ratioGrowthRate = generationConfig->contractionRate;
        growthRateUpdatesCount = 0;
        equilibrationsCount = 0;
        startTime = clock();

        CreateEventProviders();
        CreateEventProcessors();

        ParticlePair closestPair = closestPairProvider->FindClosestPair();
        innerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
        if (preserveInitialDiameter)
        {
            if (innerDiameterRatio > 1.0)
            {
                innerDiameterRatio = 1.0;
            }
            if (innerDiameterRatio < 1.0 - 1e-7)
            {
                printf("Inner diameter ratio is %.15f\n", innerDiameterRatio);
                throw InvalidOperationException("preserveInitialDiameter is true, \
but innerDiameterRatio is < 1.0, so preserving will lead to particle intersections, \
decreasing the diameters will lead to unexpected consequences (e.g., incorrect density).");
            }
        }

        initialInnerDiameterRatio = innerDiameterRatio;
        currentTime = 0;
        statistics.collisionErrorsExisted = false;

        particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);

        movingParticles.resize(config->particlesCount);
        Particle::CopyPackingTo(*particles, &movingParticles);
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
             movingParticles[particleIndex].lastEventTime = 0;
        }

        velocityService.FillVelocities(&movingParticles);

        InitializeEvents();
        shouldContinue = true;
    }

    void LubachevsckyStillingerStep::CreateEventProviders()
    {
        eventProviders.clear();

        // Providers
        boost::shared_ptr<IEventProvider> collisionEventProvider(new CollisionEventProvider(&particleCollisionService, neighborProvider));
        boost::shared_ptr<IEventProvider> neighborTransferEventProvider(new NeighborTransferEventProvider(neighborProvider, &velocityService));
        boost::shared_ptr<IEventProvider> wallTransferEventProvider(new WallTransferEventProvider(&velocityService, &geometryCollisionService, *config));

        eventProviders.push_back(collisionEventProvider);
        eventProviders.push_back(neighborTransferEventProvider);
        eventProviders.push_back(wallTransferEventProvider);

        if (lockParticles)
        {
            VoronoiTesselationProvider tesselationProvider(&geometryCollisionService, mathService, generationConfig->baseFolder, particles, config->particlesCount);
//            VoronoiTesselationProvider tesselationProvider(&geometryCollisionService, mathService, "", particles, config->particlesCount);
            tesselationProvider.FillVoronoiTesselation(&voronoiTesselation);

            boost::shared_ptr<IEventProvider> voronoiTransferEventProvider(new VoronoiTransferEventProvider(&geometryCollisionService, &velocityService, mathService, &voronoiTesselation));
//            voronoiTransferEventProvider->reflectFromSpheres = true;
            eventProviders.push_back(voronoiTransferEventProvider);
        }

        MemoryUtility::RemoveOwnership(eventProviders, &eventProvidersNotOwned);

        CompositeEventProvider::OptimizeEventProviders(&eventProvidersNotOwned);
        eventProvider.reset(new CompositeEventProvider(&eventsQueue, &eventProvidersNotOwned));
    }

    void LubachevsckyStillingerStep::CreateEventProcessors()
    {
        eventProcessors.clear();

        // Processors
        collisionEventProcessor.reset(new CollisionEventProcessor(eventProvider.get(), &particleCollisionService));
        boost::shared_ptr<IEventProcessor> neighborTransferEventProcessor(new NeighborTransferEventProcessor(eventProvider.get(), neighborProvider, particles));
        boost::shared_ptr<IEventProcessor> wallTransferEventProcessor(new WallTransferEventProcessor(eventProvider.get(), *config));
        boost::shared_ptr<IEventProcessor> moveEventProcessor(new MoveEventProcessor(eventProvider.get()));

        eventProcessors.push_back(collisionEventProcessor);
        eventProcessors.push_back(neighborTransferEventProcessor);
        eventProcessors.push_back(wallTransferEventProcessor);
        eventProcessors.push_back(moveEventProcessor);

        if (lockParticles)
        {
            boost::shared_ptr<IEventProcessor> voronoiTransferEventProcessor(new VoronoiTransferEventProcessor(eventProvider.get(), &voronoiTesselation));
//            voronoiTransferEventProcessor->reflectFromSpheres = true;
            eventProcessors.push_back(voronoiTransferEventProcessor);
        }

        MemoryUtility::RemoveOwnership(eventProcessors, &eventProcessorsNotOwned);
        eventProcessor.reset(new CompositeEventProcessor(eventProcessorsNotOwned, &velocityService));
    }

    // See Lubachevsky (1990) How to Simulate Billiards and Similar Systems for events initialization scheme.
    void LubachevsckyStillingerStep::InitializeEvents()
    {
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            MovingParticle* movingParticle = &movingParticles[particleIndex];
            movingParticle->nextAvailableEvent = Event::Invalid;
            movingParticle->nextAvailableEvent.time = currentTime;
            movingParticle->nextAvailableEvent.particleIndex = movingParticle->index;
            movingParticle->nextAvailableEvent.type = EventType::Move;
        }

        MovingParticleComparer comparer;
        eventsQueue.Initialize(&movingParticles, comparer);

        vector<ParticleIndex> updatedParticleIndexes;
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            MovingParticle* movingParticle = &movingParticles[particleIndex];
            eventProvider->SetNextEvents(&movingParticles, movingParticle->nextAvailableEvent);
        }
    }

    void LubachevsckyStillingerStep::DisplaceParticles()
    {
        FLOAT_TYPE previousPressure = statistics.reducedPressure;
        ProcessEvents(eventsPerParticle * config->particlesCount);

        // The order of the calls below is very crucial, they are very interdependent! TODO: refactor somehow.
        velocityService.RescaleVelocities(currentTime, statistics.kineticEnergy, &movingParticles);

        // Double precision epsilon is 2e-16, but for making sure that no finite precision errors occur (e.g. when adding 1e-16 time lapse to global time = 1) we reset time after each generation.
        ResetTime();

        // As far as all the velocities have changed, we should recalculate all the events from scratch
        // (it's incorrect simply to rescale event times proportionally assuming that all the events will occur at the same locations,
        // as far as relative velocity-radii growth rates have also changed, so even collision locations will change).
        InitializeEvents();

        DisplaceRealParticles();

        if (generationConfig->generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerEquilibrationBetweenCompressions)
        {
            SwitchCompressionRateWithZero(previousPressure);
        }
        else if (generationConfig->generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerGradualDensification)
        {
            DecreaseCompressionRate();
        }
        else if (generationConfig->generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerConstantPower)
        {
            EnsureConstantPower();
        }
        else if (generationConfig->generationAlgorithm == PackingGenerationAlgorithm::LubachevskyStillingerBiazzo)
        {
            DecreaseCompressionRateAsBiazzo();
        }
        else
        {
            shouldContinue = statistics.reducedPressure < maxPressure;
        }
    }

    void LubachevsckyStillingerStep::EnsureConstantPower()
    {
        FLOAT_TYPE finalCompressionRate = generationConfig->finalContractionRate;
        const FLOAT_TYPE expectedFinalDensity = 0.64;
        FLOAT_TYPE initialDensity = 1.0 - CalculateCurrentPorosity(initialInnerDiameterRatio);
        FLOAT_TYPE currentDensity = 1.0 - CalculateCurrentPorosity(innerDiameterRatio);

        FLOAT_TYPE finalDensity = (initialDensity > expectedFinalDensity) ? initialDensity : expectedFinalDensity;

        FLOAT_TYPE nextRatioGrowthRate = finalCompressionRate * maxPressure / statistics.reducedPressure * pow(currentDensity / finalDensity, 4.0 / 3.0);
        bool shouldChange = nextRatioGrowthRate < ratioGrowthRate && nextRatioGrowthRate >= finalCompressionRate * 0.5;
        shouldContinue = !(nextRatioGrowthRate <= finalCompressionRate && statistics.reducedPressure >= maxPressure);
        if (shouldChange)
        {
            ratioGrowthRate = nextRatioGrowthRate;
            particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);
            InitializeEvents();

            ExecutionConfig actualExecutionConfig;
            actualExecutionConfig.systemConfig.MergeWith(*config);
            actualExecutionConfig.generationConfig.MergeWith(*generationConfig);
            actualExecutionConfig.generationConfig.contractionRate = ratioGrowthRate;

            string configPath = Path::Append(generationConfig->baseFolder, Generation::CONFIG_FILE_NAME);
            packingSerializer->SerializeConfig(configPath, actualExecutionConfig);

            printf("ratioGrowthRate: %g\n", ratioGrowthRate);
        }
    }

    void LubachevsckyStillingerStep::DecreaseCompressionRateAsBiazzo()
    {
        shouldContinue = !(statistics.reducedPressure > maxPressure && ratioGrowthRate <= generationConfig->finalContractionRate);

        // Original paper: Biazzo et al (2009) Theory of Amorphous Packings of Binary Mixtures of Hard Spheres.pdf.
        // Last values are added to avoid finite precision effects for further threshold updates.
//        boost::array<FLOAT_TYPE, 3> pressureThresholds = {1e2, 1e3, 1e9, 1e12}; // maxPressure = 1e12
//        boost::array<FLOAT_TYPE, 3> nextRatioGrowthRates = {1e-3, 1e-4, 1e-5, 1e-6}; // starting at 1e-2

//        VectorUtilities::MultiplyByValue(pressureThresholds, maxPressure / 1e12, &pressureThresholds);
//        VectorUtilities::MultiplyByValue(nextRatioGrowthRates, generationConfig->finalContractionRate / 1e-5, &nextRatioGrowthRates);

        boost::array<FLOAT_TYPE, 4> pressureThresholds = {{1e2, 1e3, 1e9, 1e12}}; // maxPressure = 1e12
        boost::array<FLOAT_TYPE, 4> nextRatioGrowthRates = {{1e-2, 1e-3, 1e-4, 1e-4 * 0.9}};

        Nullable<size_t> firstLargerPressureIndex = StlUtilities::GetLowerBoundIndex(pressureThresholds, statistics.reducedPressure);
        if (!firstLargerPressureIndex.hasValue)
        {
            // No larger pressure exists
            firstLargerPressureIndex.value = pressureThresholds.size();
        }
        int nextRatioGrowthRateIndex = firstLargerPressureIndex.value - 1;
        if (nextRatioGrowthRateIndex < 0)
        {
            return;
        }
        FLOAT_TYPE nextRatioGrowthRate = nextRatioGrowthRates[nextRatioGrowthRateIndex];
        // nextRatioGrowthRate may become greater than actual ratioGrowthRate if the pressure drops very rapidly after rate decrease.
        if (nextRatioGrowthRate >= ratioGrowthRate)
        {
            return;
        }

        ratioGrowthRate = nextRatioGrowthRate;
        particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);
        InitializeEvents();

        ExecutionConfig actualExecutionConfig;
        actualExecutionConfig.systemConfig.MergeWith(*config);
        actualExecutionConfig.generationConfig.MergeWith(*generationConfig);
        actualExecutionConfig.generationConfig.contractionRate = ratioGrowthRate;

        string configPath = Path::Append(generationConfig->baseFolder, Generation::CONFIG_FILE_NAME);
        packingSerializer->SerializeConfig(configPath, actualExecutionConfig);

        printf("ratioGrowthRate: %g\n", ratioGrowthRate);
    }

    void LubachevsckyStillingerStep::DecreaseCompressionRate()
    {
        // Try to generate RCP packings with equilibration and exponential compression rate decrease
        // Update growth rate

        bool growthRateUpdated = false;
        shouldContinue = true;
        if (ratioGrowthRate > 0.0)
        {
            shouldContinue = !(statistics.reducedPressure > maxPressure && ratioGrowthRate <= generationConfig->finalContractionRate);
            if (statistics.reducedPressure > maxPressure) // Suppress growth if the algorithm would terminate by non-equilibrium pressure
            {
                // Suppress growth to equilibrate packings
                printf("Suppress growth\n");
                ratioGrowthRate = 0.0;
                equilibrationsCount = 0;
                growthRateUpdated = true;
            }
        }
        else
        {
            // Equilibrate until pressure is below maxPressure, and start growing again
            if (statistics.reducedPressure < maxPressure)
            {
                // Also decrease the contraction rate
                growthRateUpdatesCount++;
                ratioGrowthRate = generationConfig->contractionRate / pow(generationConfig->contractionRateDecreaseFactor, growthRateUpdatesCount);

                printf("Start growing again. Compression rate is %g\n", ratioGrowthRate);
                ExecutionConfig actualExecutionConfig;
                actualExecutionConfig.systemConfig.MergeWith(*config);
                actualExecutionConfig.generationConfig.MergeWith(*generationConfig);
                actualExecutionConfig.generationConfig.contractionRate = ratioGrowthRate;

                string configPath = Path::Append(generationConfig->baseFolder, Generation::CONFIG_FILE_NAME);
                packingSerializer->SerializeConfig(configPath, actualExecutionConfig);

                if (MpiManager::GetInstance()->GetNumberOfProcesses() > 1)
                {
                    // When there are many more packings than tasks, it's better to restrict computation time,
                    // as there are always several packings (~10%) with extremely long generation times.
                    FLOAT_TYPE computationTime = (clock() - startTime) / static_cast<double>(CLOCKS_PER_SEC);
                    FLOAT_TYPE maxComputationTime = 6 * 3600; // 6 hours
                    if (computationTime > maxComputationTime)
                    {
                        shouldContinue = false;
                        printf("Computation time in a parallel environment for a current packing is too large, it may prevent other packings from processing. Terminating...");
                    }
                }

                growthRateUpdated = true;
            }
            else
            {
                equilibrationsCount++;
                const int maxEquilibrationsCount = 50;
                shouldContinue = equilibrationsCount < maxEquilibrationsCount;
                if (!shouldContinue)
                {
                    printf("Equilibration lasted for %d rounds, but pressure is still high. Packing is almost jammed.\n", maxEquilibrationsCount);
                    // May be throw an error or simply discard packings with low coordination number that require long equilibration:
                    // long equilibration will lead to macroscopic rearrangement.
                }
            }
        }

        // Handle growth rate update
        if (growthRateUpdated)
        {
            particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);
            InitializeEvents();
        }
    }

    void LubachevsckyStillingerStep::SwitchCompressionRateWithZero(Core::FLOAT_TYPE previousPressue)
    {
        // Try to generate RCP packings with equilibration
        // Update growth rate
        bool growthRateUpdated = false;
        shouldContinue = true;
        if (ratioGrowthRate > 0.0) // Suppress growth after each 2e4 events
        {
            if (statistics.reducedPressure > maxPressure) // Suppress growth if the algorithm would terminate by non-equilibrium pressure
            {
                // Suppress growth to equilibrate packings.
                printf("Suppress growth\n");
                ratioGrowthRate = 0.0;
                growthRateUpdated = true;
            }
        }
        else
        {
            // Equilibrate completely
            bool equilibrated = std::abs(previousPressue - statistics.reducedPressure) / statistics.reducedPressure < 1e-2;
            if (equilibrated)
            {
                printf("Packing equilibrated. Start growing again.\n");
                ratioGrowthRate = generationConfig->contractionRate;
                growthRateUpdated = true;
                shouldContinue = statistics.reducedPressure < maxPressure;
            }
        }

        // Handle growth rate update
        if (growthRateUpdated)
        {
            particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);
            InitializeEvents();
        }
    }

    void LubachevsckyStillingerStep::ProcessEvents(int count)
    {
        // Reset statistics
        eventProcessor->ResetStatistics();
        collisionEventProcessor->ResetStatistics(eventsPerParticle * config->particlesCount);

        for (int i = 0; i < count; ++i)
        {
            int nextEventParticleIndex = eventsQueue.GetTopIndex();
            Event nextEvent = movingParticles[nextEventParticleIndex].nextAvailableEvent;
            currentTime = nextEvent.time;
            eventProcessor->ProcessEvent(&movingParticles, nextEvent);
        }

        CalculateStatistics();
    }

    void LubachevsckyStillingerStep::CalculateStatistics()
    {
        // Sometimes i get negative exchanged momentum. TODO: investigate this carefully.
        statistics.exchangedMomentum = std::abs(collisionEventProcessor->exchangedMomentum);
        statistics.kineticEnergy = velocityService.GetActualKineticEnergy(movingParticles);
        statistics.eventsCount = eventsPerParticle * config->particlesCount;
        statistics.timePeriod = currentTime;

        // See Otsuki and Hayakawa (2012) Critical scaling of a jammed system after a quench of temperature, formula (3)
        // (but the kinetic energy term should be multiplied by 2 to make p = nkT for ideal gas, as E = 3 / 2 nkT).
        // Formula (3) in Otsuki and Hayakawa can be derived on a basis of Salsburg, Wood (1962) Equation of State of Classical Hard Spheres at High Density, eq. (3.2).
        // So p = <sum by all i-j pairs of r_ij f_ij> / (3 V) + 2 / (3 V) E =
        // = <sum by all i, j of r_ij f_ij> / (2 3 V) + 2 / (3 V) E =
        // = 1 / (2 3 V) [sum by all events r_ij (f_ij + f_ji) t_event] 1 / T + 2 / (3 V) E =
        // = 1 / (2 3 V) [sum by all events r_ij (m_i delta_v_i + m_j delta_v_j)] 1 / T + 2 / (3 V) E =
        // = 1 / (3 V) [sum by all events r_ij m_i delta_v_i] 1 / T + 2 / (3 V) E =
        // = 1 / (3 V) exchangedMomentum / T + 2 / (3 V) E.
        // So reduced pressure = p / [2 / (3 V) E] = 1 + exchangedMomentum / (2 T E),
        // where exchangedMomentum = sum by all events r_ij m_i delta_v_i.
        statistics.reducedPressure = 1.0 + statistics.exchangedMomentum / (2.0 * statistics.kineticEnergy * currentTime);

        printf("Time: %g, reduced pressure: %g, actual temperature: %g\n",
                currentTime, statistics.reducedPressure, velocityService.GetActualTemperature(statistics.kineticEnergy, movingParticles));

//        collisionEventProcessor->FillDistinctCollidingPairs(&collidedPairs);

//        for (int i = 0; i < EVENT_TYPES_COUNT; i++)
//        {
//            int eventCount = eventProcessor->GetEventTypeCount(EVENT_TYPES[i]);
//            printf("%s occurrence count: %d\n", EVENT_TYPE_NAMES[i], eventCount);
//        }
    }

    void LubachevsckyStillingerStep::ResetTime()
    {
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            movingParticles[particleIndex].lastEventTime -= currentTime;
            movingParticles[particleIndex].nextAvailableEvent.time -= currentTime;
        }

        innerDiameterRatio = initialInnerDiameterRatio + ratioGrowthRate * currentTime;
        initialInnerDiameterRatio = initialInnerDiameterRatio + ratioGrowthRate * currentTime;
        particleCollisionService.Initialize(initialInnerDiameterRatio, ratioGrowthRate);
        currentTime = 0;
    }

    void LubachevsckyStillingerStep::DisplaceRealParticles()
    {
        Packing& particlesRef = *particles;

        // Synchronize domain particles to be serialized correctly and to be used between the event sets.
        // May not call StartMove and EndMove for the neighborProvider, as it is being synchronized in the course of dynamics (see special event type NeighborTransfer).
        for (ParticleIndex particleIndex = 0; particleIndex < config->particlesCount; ++particleIndex)
        {
            MovingParticle& movingParticle = movingParticles[particleIndex];
            DomainParticle& particle = particlesRef[particleIndex];

            neighborProvider->StartMove(particleIndex);
            VectorUtilities::MultiplyByValue(movingParticle.velocity, currentTime - movingParticle.lastEventTime, &particle.coordinates);
            VectorUtilities::Add(movingParticle.coordinates, particle.coordinates, &particle.coordinates);
            neighborProvider->EndMove();
        }

        // Check collision errors. TODO: extract method
        statistics.collisionErrorsExisted = false;
        innerDiameterRatio = initialInnerDiameterRatio + ratioGrowthRate * currentTime;

        // TODO: exclude pairs of immobile particles from comparison. They can grow and eventually overlap.
        ParticlePair closestPair = closestPairProvider->FindClosestPair();
        FLOAT_TYPE expectedInnerDiameterRatio = sqrt(closestPair.normalizedDistanceSquare);
        if (expectedInnerDiameterRatio < innerDiameterRatio - 1e-14) // (std::abs(expectedInnerDiameterRatio - innerDiameterRatio) > 1e-14)
        {
            printf("WARNING: innerDiameterRatio incorrect. Time: %g. Actual: %1.15f, expected: %1.15f. Closest pair: %d, %d\n",
                                currentTime, innerDiameterRatio, expectedInnerDiameterRatio, closestPair.firstParticleIndex, closestPair.secondParticleIndex);
            statistics.collisionErrorsExisted = true;
        }

//        // For debug purposes only!
//        closestPair = geometryService->GetMinNormalizedDistanceNaive(*particles);
//        FLOAT_TYPE minNormalizedDistance = sqrt(closestPair.normalizedDistanceSquare);
//
//        if (expectedInnerDiameterRatio < innerDiameterRatio - 1e-14)
//        {
//            printf("ERROR: innerDiameterRatio %g is not equal to min normalized distance from naive computation %g in the pair %d %d. Probably bugs in distance provider.\n",
//                    innerDiameterRatio, minNormalizedDistance, closestPair.firstParticleIndex, closestPair.secondParticleIndex);
//            throw InvalidOperationException("InnerDiameterRatio is not equal to min normalized distance from naive computation.");
//        }
    }

    bool LubachevsckyStillingerStep::ShouldContinue() const
    {
        return shouldContinue;
    }

    void LubachevsckyStillingerStep::ResetGeneration()
    {
        throw InvalidOperationException("ResetGeneration does nothing for LubachevsckyStillingerStep. Always set maxRunsCount = 1 for LS.");
    }
}

