// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_DistanceServices_Headers_VerletListNeighborProvider_h
#define Generation_PackingServices_DistanceServices_Headers_VerletListNeighborProvider_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "INeighborProvider.h"
namespace PackingServices { class GeometryCollisionService; }
namespace PackingServices { class GeometryService; }
namespace Model { class SystemConfig; }
namespace PackingServices { class MathService; }

namespace PackingServices
{
    // Represents a class to efficiently determine nearest neighbors of a current particle.
    // Uses Verlet Lists algorithm: see http://en.wikipedia.org/wiki/Verlet_list and Raschdorf, Kolonko (2011) A comparison of data structures for the simulation of polydisperse particle packings.
    // Essentially it's a decorator over a baseNeighborProvider.

    // NOTES:
    // Definition: Verlet lists are correct if all particle intersections can be obtained from Verlet lists
    // 1. If all particle centers remain in the half-cutoff radius, the verlet lists are correct
    // 2. when a particle (#1) center leaves the half-cutoff sphere, we update its verlet list, but add those particles (#2),
    // which intersect the #1 verlet sphere as if they were in their verlet sphere centers, not in actual positions
    // It's equivalent to a. moving the particle #2 back to its verlet center, b. moving the particle #1 outside its half-cutoff sphere c. updating the verlet list of #1 d. moving particle 2 back.
    // According to 1. the verlet lists will be correct after these manipulations.
    // 3. If we try to add particles according to their actual positions, errors may occur. E.g. the particle 2 is too far away from 1 (but is still in its half-cutoff sphere), and does not intersect its verlet sphere.
    // Still, if the particle 2 returns to its initial position, it may intersect the new verlet sphere of particle 1, but this will not be reflected in verlet lists, and verlet lists will be incorrect.
    // 4. Therefore, we need to pass verlet list centers to base neighbor provider, not actual particle centers. That's why we need verletParticles array. Also, this allows updating the base neighbor provider much rarely.
    // 5. To make base neighbor provider not to miss neighbors, suitable for verlet lists, we need to pass particle with radii r_i + cutoffDistance / 2 to baseNeighborProvider.
    class VerletListNeighborProvider : public virtual INeighborProvider
    {
    private:
        // Services
        GeometryService* geometryService;
        INeighborProvider* baseNeighborProvider;
        MathService* mathService;
        GeometryCollisionService* geometryCollisionService;

        // Main variables
        const Model::Packing* particles;
        const Model::SystemConfig* config;

        // Can't introduce VerletParticle, a sublcass of a Model::DomainParticle, as we operate (and pass) an array Model::DomainParticle* to baseNeighborProvider, not an array of pointers.
        Model::Packing verletParticles; // Represents an array of particles with coordinates as centers of verlet spheres. It should be passed to baseNeighborProvider.
        std::vector<std::vector<Model::ParticleIndex> > verletLists; // An array of Verlet lists for a single particle.

        Model::ParticleIndex movedParticleIndex;
        Core::FLOAT_TYPE cutoffDistance;
        mutable bool cutoffDistanceSet;

    public:
        static const Core::FLOAT_TYPE MAX_EXPECTED_OUTER_DIAMETER_RATIO;

    public:
        VerletListNeighborProvider(GeometryService* geometryService, MathService* mathService, INeighborProvider* baseNeighborProvider, GeometryCollisionService* geometryCollisionService);

        virtual ~VerletListNeighborProvider();

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        OVERRIDE void SetParticles(const Model::Packing& particles);

        void SetCutoffDistance(Core::FLOAT_TYPE cutoffDistance);

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(Model::ParticleIndex particleIndex, Model::ParticleIndex* neighborsCount) const;

        OVERRIDE const Model::ParticleIndex* GetNeighborIndexes(const Core::SpatialVector& coordinates, Model::ParticleIndex* neighborsCount) const;

        // Returns a negative number if the boundary is never crossed.
        OVERRIDE Core::FLOAT_TYPE GetTimeToUpdateBoundary(Model::ParticleIndex particleIndex, const Core::SpatialVector& point, const Core::SpatialVector& velocity) const;

        OVERRIDE void StartMove(Model::ParticleIndex particleIndex);

        OVERRIDE void EndMove();

    private:
        void SpreadParticlesByLists();

        void FillVerletList(Model::ParticleIndex particleIndex);

        bool IsParticleInVerletSphere(Model::ParticleIndex verletListIndex, Model::ParticleIndex particleIndex) const;

        void AddParticleToVerletList(Model::ParticleIndex verletListIndex, Model::ParticleIndex particleIndex);

        void RemoveParticleFromVerletList(Model::ParticleIndex verletListIndex, Model::ParticleIndex particleIndex);

        DISALLOW_COPY_AND_ASSIGN(VerletListNeighborProvider);
    };
}

#endif /* Generation_PackingServices_DistanceServices_Headers_VerletListNeighborProvider_h */

