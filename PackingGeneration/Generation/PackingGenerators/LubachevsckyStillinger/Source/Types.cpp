// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "Generation/PackingGenerators/LubachevsckyStillinger/Headers/Types.h"
#include "Core/Headers/Constants.h"

namespace PackingGenerators
{
    const Event Event::Invalid = Event();

    Event::Event()
    {
        type = EventType::InvalidEvent;
        particleIndex = Event::InvalidIndex;
        neighborIndex = Event::InvalidIndex;
        time = Core::MAX_FLOAT_VALUE;
        wallIndex = Event::InvalidIndex;
    }

    const EventType::Type EVENT_TYPES[] = {EventType::InvalidEvent, EventType::Move, EventType::Collision, EventType::WallTransfer, EventType::NeighborTransfer, EventType::VoronoiInscribedSphereTransfer, EventType::VoronoiTransfer};
    const char* const EVENT_TYPE_NAMES[] = {"InvalidEvent", "Move", "Collision", "WallTransfer", "NeighborTransfer", "VoronoiInscribedSphereTransfer", "VoronoiTransfer"};
}

