// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VoronoiTransferEventProcessor.h"

#include "Core/Headers/Exceptions.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "../Headers/IEventProvider.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    VoronoiTransferEventProcessor::VoronoiTransferEventProcessor(IEventProvider* eventProvider,
            vector<VoronoiPolytope>* voronoiTesselation) : BaseEventProcessor(eventProvider)
    {
        this->voronoiTesselation = voronoiTesselation;
        reflectFromSpheres = false;
    }

    void VoronoiTransferEventProcessor::ProcessEventSafe(Event event)
    {
        ParticleIndex particleIndex = event.particleIndex;
        vector<MovingParticle>& movingParticlesRef = *movingParticles;
        MovingParticle& particle = movingParticlesRef[particleIndex];

        if (!reflectFromSpheres)
        {
            if (event.type == EventType::VoronoiInscribedSphereTransfer)
            {
                // Do nothing with the sphere, just search for the next event
                eventProvider->SetNextEvents(movingParticles, event);
            }

            VoronoiPolytope& polytope = voronoiTesselation->at(particleIndex);
            Plane& plane = polytope.planes[event.wallIndex];

            // Test
#ifdef DEBUG
            GeometryCollisionService geometryCollisionService;
            FLOAT_TYPE distanceToPlane = geometryCollisionService.GetDistance(particle.coordinates, plane);
            if (std::abs(distanceToPlane) > 1e-6)
            {
                throw InvalidOperationException("Collision with the Voronoi plane happens far away from the plane. Something is very wrong.");
            }
#endif

            // Specular reflection from the Voronoi plane. See http://en.wikipedia.org/wiki/Specular_reflection
            // Note that the formula in unit vectors can be multiplied by velocity length, and initial direction may not be a unit vector.
            // Also note that if the plane normal changes sign, it is multiplied by itself, so the normal direction doesn't matter.
            // But!: by this link d_i is the direction from the surface, which is opposite to the velocity, so we multiply it by -1
            FLOAT_TYPE dotProduct = VectorUtilities::GetDotProduct(particle.velocity, plane.normal);

            SpatialVector tempVelocity;
            VectorUtilities::MultiplyByValue(plane.normal, - 2.0 * dotProduct, &tempVelocity);
            VectorUtilities::Add(tempVelocity, particle.velocity, &particle.velocity);
            eventProvider->SetNextEvents(movingParticles, event);

//        // Bounce-back
//        SpatialVector newVelocity;
//        VectorUtilities::Multiply(particle->velocity, - 1.0, newVelocity);
//        UpdateVelocityAfterCollision(*event, newVelocity);

//        printf("Voronoi reflection: particle index: %d; coordinates: %f %f %f; velocity: %f %f %f; wall index: %d\n",
//                particleIndex,
//                particle->coordinates[Axis::X],
//                particle->coordinates[Axis::Y],
//                particle->coordinates[Axis::Z],
//                particle->velocity[Axis::X],
//                particle->velocity[Axis::Y],
//                particle->velocity[Axis::Z],
//                event->wallIndex);
        }
        else
        {
            // Bounce-back
            VectorUtilities::MultiplyByValue(particle.velocity, - 1.0, &particle.velocity);
            eventProvider->SetNextEvents(movingParticles, event);
        }
    }

    bool VoronoiTransferEventProcessor::ShouldProcessEvent(Event event) const
    {
        return event.type == EventType::VoronoiInscribedSphereTransfer || event.type == EventType::VoronoiTransfer;
    }
}

