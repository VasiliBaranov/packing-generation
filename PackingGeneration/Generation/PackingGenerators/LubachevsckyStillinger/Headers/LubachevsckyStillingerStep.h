// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_LubachevsckyStillingerStep_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_LubachevsckyStillingerStep_h

#include <ctime>
#include <boost/shared_ptr.hpp>
#include "Core/Headers/OrderedPriorityQueue.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingGenerators/Headers/BasePackingStep.h"
#include "Types.h"
#include "ParticleCollisionService.h"
#include "VelocityService.h"
namespace PackingGenerators { class CompositeEventProcessor; }
namespace PackingGenerators { class CollisionEventProcessor; }
namespace PackingGenerators { class CompositeEventProvider; }
namespace PackingGenerators { class IEventProvider; }
namespace PackingGenerators { class IEventProcessor; }
namespace PackingServices { class PackingSerializer; }
namespace PackingServices { class IEnergyService; }

namespace PackingGenerators
{
    // See Lubachevscky, Stillinger (1990) Geometric properties of random disk packings
    // or Lubachevsky (1990) How to Simulate Billiards and Similar Systems.
    // TODO: refactor!
    class LubachevsckyStillingerStep : public BasePackingStep
    {
    public:
        // TODO: move this class to statistics, and improve architecture: may be calculate the entire statistics in composite event processor, expose statistics.
        std::vector<CollidingPair> collidedPairs;
        Model::MolecularDynamicsStatistics statistics;
        int eventsPerParticle;
        bool lockParticles;
        bool preserveInitialDiameter;

    private:
        // Original Donev code usually terminates at 1e12, but for those packings that exhibit 1e12 for Donev code our code determines pressure at 1e8.
        // Possible reasons for pressure discrepancy with original LS code are:
        // 1. different (and probably incorrect in Donev LS code) collision time computation
        // 2. Events for particles that were going to collide with the particles in the current collision pair are not updated in the original code
        // 3. ! Velocities are not rescaled after each 20 000 events, so temperature always grows.
        // These are also the most probable reasons for PSD generation errors of the closed source program of Donev.
        static const Core::FLOAT_TYPE maxPressure;

        // Working variables
        Core::FLOAT_TYPE initialInnerDiameterRatio;
        Core::FLOAT_TYPE currentTime;
        Core::FLOAT_TYPE ratioGrowthRate;
        std::vector<MovingParticle> movingParticles;
        bool shouldContinue;
        int growthRateUpdatesCount;
        int equilibrationsCount;
        clock_t startTime;

        // Services
        // These ones are too internal for the LS Step, so i do not pass them in constructor as pointers.
        Core::OrderedPriorityQueue<std::vector<MovingParticle>, MovingParticleComparer> eventsQueue;
        ParticleCollisionService particleCollisionService;
        VelocityService velocityService;
        PackingServices::GeometryCollisionService geometryCollisionService;
        PackingServices::PackingSerializer* packingSerializer;
        PackingServices::IEnergyService* contractionEnergyService;

        // Event providers and processors. Most of these fields are needed for memory management.
        // I'm initializing the providers and processors inside this class, as it knows better which providers and processors it needs.
        std::vector<boost::shared_ptr<IEventProvider> > eventProviders; // this vector is needed to own eventProviders
        std::vector<IEventProvider*> eventProvidersNotOwned; // this vector is needed, as far as CompositeEventProvider accepts a vector of pointers but doesn't own it
        std::vector<boost::shared_ptr<IEventProcessor> > eventProcessors;
        std::vector<IEventProcessor*> eventProcessorsNotOwned;

        boost::shared_ptr<CompositeEventProvider> eventProvider;
        boost::shared_ptr<CompositeEventProcessor> eventProcessor;
        boost::shared_ptr<CollisionEventProcessor> collisionEventProcessor;
        std::vector<VoronoiPolytope> voronoiTesselation;

    public:
        LubachevsckyStillingerStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::IClosestPairProvider* distanceService,
                PackingServices::MathService* mathService,
                PackingServices::PackingSerializer* packingSerializer,
                PackingServices::IEnergyService* contractionEnergyService);

        OVERRIDE void SetParticles(Model::Packing* particles);

        OVERRIDE void DisplaceParticles();

        OVERRIDE void ResetGeneration();

        OVERRIDE bool ShouldContinue() const;

        ~LubachevsckyStillingerStep();

    private:
        void ProcessEvents(int count);

        void DecreaseCompressionRate();

        void DecreaseCompressionRateAsBiazzo();

        void EnsureConstantPower();

        void SwitchCompressionRateWithZero(Core::FLOAT_TYPE previousPressue);

        void CreateEventProviders();

        void CreateEventProcessors();

        void InitializeEvents();

        void ResetTime();

        void DisplaceRealParticles();

        void CalculateStatistics();

        DISALLOW_COPY_AND_ASSIGN(LubachevsckyStillingerStep);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_LubachevsckyStillingerStep_h */

