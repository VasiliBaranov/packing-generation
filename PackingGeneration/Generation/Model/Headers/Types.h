// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_Model_Headers_Types_h
#define Generation_Model_Headers_Types_h

#include <vector>
#include "Core/Headers/Types.h"
#include "Core/Headers/Exceptions.h"

// The basic project structure is from the Domain-Driven Design: Model (merely data transfer objects) and Services (see Domain-Driven Design Quickly).
// Several model types are included in this file (Types.h) for simplicity, services include PackingGenerators and PackingServices (i.e. services over packings).
// To reduce folder depth we omit the parent "Services" folder.
namespace Model
{
    typedef int ParticleIndex;

    // Represents a single particle
    struct Particle
    {
        Core::SpatialVector coordinates;
        Core::FLOAT_TYPE diameter;

        Particle()
        {

        }

        Particle(Core::FLOAT_TYPE diameter, const Core::SpatialVector& coordinates)
        {
            this->diameter = diameter;
            this->coordinates = coordinates;
        }

        // Don't make it virtual, as it is much better to have strongly typed methods in each subclass.
        void CopyTo(Particle* target) const
        {
            target->coordinates = coordinates;
            target->diameter = diameter;
        }

        template<class TContainerSource, class TContainerTarget>
        static void CopyPackingTo(const TContainerSource& sourcePacking, TContainerTarget* targetPacking)
        {
            CopyPackingTo(sourcePacking.begin(), targetPacking->begin(), sourcePacking.end() - sourcePacking.begin());
        }

    private:
        template<class TSourceRandomAccessIterator, class TTargetRandomAccessIterator>
        static void CopyPackingTo(TSourceRandomAccessIterator sourcePacking, TTargetRandomAccessIterator targetPacking, int particlesCount)
        {
            for (ParticleIndex i = 0; i < particlesCount; i++)
            {
                sourcePacking[i].CopyTo(&targetPacking[i]);
            }
        }
    };

    // Represents a single particle in the domain
    struct DomainParticle : public Particle
    {
        // Defines global index in the particle list
        ParticleIndex index;
        bool isImmobile; // denotes fixed particles or particles outside the active geometry

        DomainParticle()
        {
            isImmobile = false;
        }

        DomainParticle(ParticleIndex index, Core::FLOAT_TYPE diameter, const Core::SpatialVector& coordinates) : Particle(diameter, coordinates)
        {
            this->index = index;
            isImmobile = false;
        }

        // Don't make it virtual, as it is much better to have strongly typed methods in each subclass.
        void CopyTo(DomainParticle* target) const
        {
            target->index = index;
            target->isImmobile = isImmobile;
            Particle::CopyTo(target);
        }
    };

    typedef std::vector<DomainParticle> Packing;

    struct ParticlePair
    {
        ParticleIndex firstParticleIndex;
        ParticleIndex secondParticleIndex;
        Core::FLOAT_TYPE normalizedDistanceSquare;

        ParticlePair()
        {
            firstParticleIndex = -1;
            secondParticleIndex = -1;
            normalizedDistanceSquare = -1;
        }

        ParticlePair(ParticleIndex firstParticleIndex, ParticleIndex secondParticleIndex, Core::FLOAT_TYPE normalizedDistanceSquare)
        {
            this->firstParticleIndex = firstParticleIndex;
            this->secondParticleIndex = secondParticleIndex;
            this->normalizedDistanceSquare = normalizedDistanceSquare;
        }

        ParticleIndex GetOtherIndex(ParticleIndex particleIndex) const
        {
            if (particleIndex == firstParticleIndex)
            {
                return secondParticleIndex;
            }
            else if (particleIndex == secondParticleIndex)
            {
                return firstParticleIndex;
            }
            else
            {
                throw Core::InvalidOperationException("particleIndex should be one of the pair indexes.");
            }
        }
    };

    struct BoundariesMode
    {
        enum Type
        {
            Unknown = 0,
            Bulk = 1,
            Ellipse = 2,
            Rectangle = 3,
            Trapezoid = 4
        };
    };

    struct InitialParticleDistribution
    {
        enum Type
        {
            Unknown = 0,
            Poisson = 1, // R-packings from S. Khirevich papers
            PoissonInCells = 2 // S-packings from S. Khirevich papers
        };
    };

    struct PackingGenerationAlgorithm
    {
        // Defines different algorithms of particle generation.
        enum Type
        {
            Unknown = 0,

            LubachevskyStillingerSimple = 1,
            LubachevskyStillingerGradualDensification = 2,
            LubachevskyStillingerEquilibrationBetweenCompressions = 3, // See V. Baranau et. al. (2013) Pore-size entropy of random hard-sphere packings, Soft Matter

            ForceBiasedAlgorithm = 4,
            OriginalJodreyTory = 5,
            KhirevichJodreyTory = 6, // See dissertation by S. Khirevich and his papers
            MonteCarlo = 7,
            ConjugateGradient = 8,
            ClosestJammingSearch = 9,

            LubachevskyStillingerConstantPower = 10,
            LubachevskyStillingerBiazzo = 11
        };
    };

    struct ExecutionMode
    {
        // Defines different execution modes for program operation.
        enum Type
        {
            Unknown = 0,
            PackingGeneration = 1,
            InsertionRadiiGeneration = 2,
            DirectionsCalculation = 3,
            ContractionEnergyCalculation = 4,
            OrderCalculation = 5,
            EntropyCalculation = 6,
            HessianEigenvaluesCalculation = 7,
            PressureCalculation = 8,
            MolecularDynamicsCalculation = 9,
            RattlerRemoval = 10,
            PairCorrelationCalculation = 11,
            StructureFactorCalculation = 12,
            LocalOrientationalDisorder = 13,
            ImmediateMolecularDynamicsCalculation = 14,
            DistancesToClosestSurfacesCalculation = 15,
            ContactNumberDistributionCalculation = 16,
            NearestNeighborsCalculation = 17,
            ActiveGeometryCalculation = 18,
            SuccessfulPermutationProbability = 19
        };
    };

    // This class belongs more to the Types.h in LubachevskyStillinger folder, but PackingSerializer requires it also, so i moved it here.
    struct MolecularDynamicsStatistics
    {
        Core::FLOAT_TYPE reducedPressure;
        Core::FLOAT_TYPE exchangedMomentum;
        Core::FLOAT_TYPE kineticEnergy;
        int eventsCount;
        Core::FLOAT_TYPE timePeriod;
        bool collisionErrorsExisted;

        // Needed just for gathering MD statistics for existing packings
        unsigned long long equilibrationEventsCount;
    };

    struct PressureData
    {
        Core::FLOAT_TYPE pressure;
        Core::FLOAT_TYPE contractionRatio;
        Core::FLOAT_TYPE density;
        Core::FLOAT_TYPE jammingDensity;
        bool collisionErrorsExisted;
    };

    struct PackingInfo
    {
        Core::FLOAT_TYPE theoreticalPorosity;
        Core::FLOAT_TYPE calculatedPorosity;
        Core::FLOAT_TYPE tolerance;
        Core::FLOAT_TYPE totalTime;
        unsigned long long iterationsCount;
    };

    // Represents a plane that can only be perpendicular to Cartesian axes
    struct SimplePlane
    {
        Core::Axis::Type perpendicularAxis; // Axis, perpendicular to the wall
        int outerNormalDirection; // -1 or +1. Outer Normal projection to the axis
        Core::FLOAT_TYPE coordinateOnAxis;
    };

    struct CubicBox
    {
        static const int wallsCount = DIMENSIONS * 2;
        Model::SimplePlane walls[wallsCount];
        int oppositWallIndexes[wallsCount];

        void Initialize(const Core::SpatialVector& minVertexCoordinate, const Core::SpatialVector& boxSize)
        {
            for (int dimension = 0; dimension < DIMENSIONS; ++dimension)
            {
                int normalIndex = 2 * dimension;
                walls[normalIndex].perpendicularAxis = static_cast<Core::Axis::Type>(dimension);
                walls[normalIndex].outerNormalDirection = 1;
                walls[normalIndex].coordinateOnAxis = minVertexCoordinate[dimension] + boxSize[dimension];

                normalIndex++;
                walls[normalIndex].perpendicularAxis = static_cast<Core::Axis::Type>(dimension);
                walls[normalIndex].outerNormalDirection = -1;
                walls[normalIndex].coordinateOnAxis = minVertexCoordinate[dimension];

                oppositWallIndexes[normalIndex - 1] = normalIndex;
                oppositWallIndexes[normalIndex] = normalIndex - 1;
            }
        }
    };

    struct Plane
    {
        Core::SpatialVector normal;
        Core::FLOAT_TYPE displacement;
    };

    // TODO: merge with CubicBox, remove code duplication in GeometryCollisionService::FindIntersection
    struct Polytope
    {
        std::vector<Plane> planes;
    };

    struct PairCorrelationFunction
    {
        std::vector<Core::FLOAT_TYPE> binLeftEdges;
        std::vector<int> binParticleCounts;
        std::vector<Core::FLOAT_TYPE> pairCorrelationFunctionValues;
    };

    struct StructureFactor
    {
        std::vector<Core::FLOAT_TYPE> waveVectorLengths;
        std::vector<Core::FLOAT_TYPE> structureFactorValues;
    };
}

#endif /* Generation_Model_Headers_Types_h */

