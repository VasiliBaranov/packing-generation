// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_Types_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_Types_h

#include "Generation/Model/Headers/Types.h"
#include "Core/Headers/VectorUtilities.h"

namespace PackingGenerators
{
    struct EventType
    {
        enum Type
        {
            InvalidEvent = 0,
            Move = 1,
            Collision = 2,
            WallTransfer = 3,
            NeighborTransfer = 4,
            VoronoiTransfer = 5,
            VoronoiInscribedSphereTransfer = 6
        };
    };

    // Enum metadata. There may be better solutions. I could also create a global map of enums metadata, not to pollute the namespace too much; enum names can be a map by itself.
    // For printing: http://stackoverflow.com/questions/201593/is-there-a-simple-script-to-convert-c-enum-to-string and http://stackoverflow.com/questions/3342726/c-print-out-enum-value-as-text
    // For iterating: http://stackoverflow.com/questions/261963/c-iterate-through-an-enum
    // For declarations, see http://stackoverflow.com/questions/6536805/initializing-a-static-const-array-of-const-strings-in-c and http://stackoverflow.com/questions/6514651/declare-large-global-array
    const int EVENT_TYPES_COUNT = 7;
    extern const EventType::Type EVENT_TYPES[];
    extern const char* const EVENT_TYPE_NAMES[];

    // At first i implemented a class hierarchy BaseEvent->CollisionEvent, TransferEvent, etc.,
    // but this required Visitor pattern for processing the events and extensive use of smart pointers
    // (to set pointers to events in MovingParticle in polymorphic manner) in the Lubachevsky-Stillinger class.
    struct Event
    {
        static const int InvalidIndex = -1;

        EventType::Type type;
        Core::FLOAT_TYPE time;
        Model::ParticleIndex particleIndex;
        Model::ParticleIndex neighborIndex;
        int wallIndex;

        Event();

        // Actually, Event() always constructs an invalid event, so any line "Event e = Event::Invalid;" will be equivalent simply to "Event e;". But this introduces a hidden dependency and the code is less readable.
        // I could have implemented a separate static method "CreateInvalidEvent", but there should be only one global Invalid event. And now it's possible to refer to it by reference (aka Flyweight pattern).
        static const Event Invalid;
    };

    struct MovingParticle : public Model::DomainParticle
    {
        Core::SpatialVector velocity;
        Core::FLOAT_TYPE lastEventTime;

        // This is the closest event for the particle, but after exlusion of those neighbors that already have next events earlier (which may also not be closest);
        // such neighbor events will of course be overriden before the collision of the current particle with the given neighbor will take place, so it's unnecessary to take them into consideration.
        // For details, see Lubachevsky (1990) How to Simulate Billiards and Similar Systems. All event providers and processors will follow this algorithm.
        Event nextAvailableEvent;
    };

    class MovingParticleComparer
    {
    public:
        bool operator()(const MovingParticle& first, const MovingParticle& second)
        {
            // It may give contradictory results for comparison of two particles with InvalidEvent types.
            if (first.nextAvailableEvent.type == EventType::InvalidEvent)
            {
                return false;
            }
            if (second.nextAvailableEvent.type == EventType::InvalidEvent)
            {
                return true;
            }

            // Invalid events (with event time < global time) will never appear in the events queue, so we do not check global time.
            return first.nextAvailableEvent.time < second.nextAvailableEvent.time;
        };
    };

    struct VoronoiPlane : Model::Plane
    {
        Model::ParticleIndex particleIndex;
        Model::ParticleIndex neighborIndex;

        bool operator==(const VoronoiPlane& other)
        {
            Core::SpatialVector difference;
            Core::VectorUtilities::Subtract(normal, other.normal, &difference);
            Core::FLOAT_TYPE differenceSquare = Core::VectorUtilities::GetSelfDotProduct(difference);

            const Core::FLOAT_TYPE epsilon = 1e-6;
            return particleIndex == other.particleIndex &&
                    neighborIndex == other.neighborIndex &&
                    differenceSquare < epsilon &&
                    std::abs(displacement - other.displacement) < epsilon;
        };
    };

    struct VoronoiPolytope : Model::Polytope
    {
        Core::SpatialVector insribedSphereCenter;
        Core::FLOAT_TYPE insribedSphereRadius;
    };

    struct CollidingPair
    {
        Model::ParticleIndex particleIndex;
        Model::ParticleIndex neighborIndex;
        int collisionsCount;

        bool operator==(const CollidingPair& other) const
        {
            return particleIndex == other.particleIndex &&
                    neighborIndex == other.neighborIndex;
        };

        bool operator<(const CollidingPair& other) const
        {
            if (particleIndex == other.particleIndex)
            {
                return neighborIndex < other.neighborIndex;
            }

            return particleIndex < other.particleIndex;
        };
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_Types_h */

