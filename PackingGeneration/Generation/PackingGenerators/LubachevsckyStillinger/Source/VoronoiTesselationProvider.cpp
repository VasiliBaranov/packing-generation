// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VoronoiTesselationProvider.h"

#include "Core/Headers/Constants.h"
#include "Core/Headers/Exceptions.h"
#include "Core/Headers/ScopedFile.h"
#include "Core/Headers/StlUtilities.h"
#include "Core/Headers/Path.h"
#include "Generation/PackingServices/Headers/GeometryCollisionService.h"
#include "Generation/PackingServices/Headers/MathService.h"

using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    VoronoiTesselationProvider::VoronoiTesselationProvider(GeometryCollisionService* geometryCollisionService,
            PackingServices::MathService* mathService,
            string baseFolder,
            const Packing* particles,
            Model::ParticleIndex particlesCount)
    {
        this->geometryCollisionService = geometryCollisionService;
        this->baseFolder = baseFolder;
        this->particles = particles;
        this->particlesCount = particlesCount;
    }

    void VoronoiTesselationProvider::FillVoronoiTesselation(std::vector<VoronoiPolytope>* voronoiTesselation) const
    {
        if (baseFolder == "")
        {
            voronoiTesselation->clear();
            voronoiTesselation->resize(particlesCount);

            const Packing& particlesRef = *particles;
            for (ParticleIndex i = 0; i < particlesCount; ++i)
            {
                VoronoiPolytope& polytope = voronoiTesselation->at(i);
                const DomainParticle& particle = particlesRef[i];
                polytope.insribedSphereCenter = particle.coordinates;
                polytope.insribedSphereRadius = 0.6;
            }

            return;
        }

        voronoiTesselation->clear();

        string cachePath = Path::Append(baseFolder, "Cache");
        string voronoiTesselationPath = Path::Append(cachePath, "mainVoronoi.txt");
        string periodicIndexesMapPath = Path::Append(cachePath, "mainPeriodicIndexesMap.txt");

        if (!Path::Exists(voronoiTesselationPath))
        {
            throw InvalidOperationException("Voronoi tesselation not found. Please prepare it by qhull prior to running this code.");
        }
        if (!Path::Exists(periodicIndexesMapPath))
        {
            throw InvalidOperationException("Periodic indexes map not found. Please prepare it prior to running this code.");
        }

        // TODO: implement creating periodic images of particles, saving periodicIndexesMap, particles in qhul format, running qhul. Currently it's done by MATLAB codes.

        vector<ParticleIndex> periodicIndexesMap;
        ReadPeriodicIndexesMap(periodicIndexesMapPath, &periodicIndexesMap);

        vector<VoronoiPlane> voronoiPlanes;
        ReadVoronoiPlanes(voronoiTesselationPath, periodicIndexesMap, &voronoiPlanes);

        vector<vector<VoronoiPlane> > voronoiPlanesPerParticle;
        FillPlanesPerParticle(voronoiPlanes, &voronoiPlanesPerParticle);

        FillVoronoiTesselation(voronoiPlanesPerParticle, voronoiTesselation);
        SetInscribedSpheres(*particles, voronoiTesselation);
    }

    void VoronoiTesselationProvider::SetInscribedSpheres(const Packing& particles, std::vector<VoronoiPolytope>* voronoiTesselation) const
    {
        int index = 0;
        for (vector<VoronoiPolytope>::iterator it = voronoiTesselation->begin(); it != voronoiTesselation->end(); ++it)
        {
            VoronoiPolytope& polytope = *it;
            SetInscribedSphere(particles[index], &polytope);

            index++;
        }
    }

    void VoronoiTesselationProvider::SetInscribedSphere(const DomainParticle& particle, VoronoiPolytope* polytope) const
    {
        FLOAT_TYPE minDistance = MAX_FLOAT_VALUE;
        for (vector<Plane>::iterator planeIterator = polytope->planes.begin(); planeIterator != polytope->planes.end(); ++planeIterator)
        {
            Plane& plane = *planeIterator;
            FLOAT_TYPE distance = geometryCollisionService->GetDistance(particle.coordinates, plane);
            distance = std::abs(distance);

            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }

        polytope->insribedSphereCenter = particle.coordinates;
        polytope->insribedSphereRadius = minDistance;
    }

    void VoronoiTesselationProvider::FillVoronoiTesselation(const vector<vector<VoronoiPlane> >& voronoiPlanesPerParticle, vector<VoronoiPolytope>* voronoiTesselation) const
    {
        for (vector<vector<VoronoiPlane> >::const_iterator polytopeIterator = voronoiPlanesPerParticle.begin(); polytopeIterator != voronoiPlanesPerParticle.end(); ++polytopeIterator)
        {
            const vector<VoronoiPlane>& particleVoronoiPlanes = *polytopeIterator;
            VoronoiPolytope polytope;

            for (vector<VoronoiPlane>::const_iterator planeIterator = particleVoronoiPlanes.begin(); planeIterator != particleVoronoiPlanes.end(); ++planeIterator)
            {
                const VoronoiPlane& voronoiPlane = *planeIterator;

                Plane plane;
                plane.normal = voronoiPlane.normal;
                plane.displacement = voronoiPlane.displacement;

                polytope.planes.push_back(plane);
            }

            voronoiTesselation->push_back(polytope);
        }
    }

    void VoronoiTesselationProvider::FillPlanesPerParticle(const vector<VoronoiPlane>& voronoiPlanes, vector<vector<VoronoiPlane> >* voronoiPlanesPerParticle) const
    {
        FillNonUniquePlanesPerParticle(voronoiPlanes, voronoiPlanesPerParticle);
        RemoveDuplicatePlanesPerParticle(voronoiPlanesPerParticle);
    }

    void VoronoiTesselationProvider::FillNonUniquePlanesPerParticle(const vector<VoronoiPlane>& voronoiPlanes, vector<vector<VoronoiPlane> >* voronoiPlanesPerParticle) const
    {
        vector<vector<VoronoiPlane> >& voronoiPlanesPerParticleRef = *voronoiPlanesPerParticle;
        voronoiPlanesPerParticleRef.resize(particlesCount);

        for (vector<VoronoiPlane>::const_iterator it = voronoiPlanes.begin(); it != voronoiPlanes.end(); ++it)
        {
            const VoronoiPlane& voronoiPlane = *it;
            voronoiPlanesPerParticleRef[voronoiPlane.particleIndex].push_back(voronoiPlane);

            VoronoiPlane neighborPlane = voronoiPlane;
            neighborPlane.particleIndex = voronoiPlane.neighborIndex;
            neighborPlane.neighborIndex = voronoiPlane.particleIndex;
            voronoiPlanesPerParticleRef[neighborPlane.particleIndex].push_back(neighborPlane);
        }
    }

    void VoronoiTesselationProvider::RemoveDuplicatePlanesPerParticle(vector<vector<VoronoiPlane> >* voronoiPlanesPerParticle) const
    {
        for (vector<vector<VoronoiPlane> >::iterator it = voronoiPlanesPerParticle->begin(); it != voronoiPlanesPerParticle->end(); ++it)
        {
            vector<VoronoiPlane>& particleVoronoiPlanes = *it;
            // This will still leave periodic images of Voronoi planes. It is intentional, as GeometryCollisionService doesn't impose periodicity when searching for intersections.
            // It makes the operation faster, because for most Voronoi cells periodic images will be far away from the box and not present, and particles will never reach such images between collisions.
            StlUtilities::SortAndResizeToUnique(&particleVoronoiPlanes);
        }
    }

    void VoronoiTesselationProvider::ReadVoronoiPlanes(string path, const vector<ParticleIndex>& periodicIndexesMap, vector<VoronoiPlane>* voronoiPlanes) const
    {
        ReadPeriodicVoronoiPlanes(path, voronoiPlanes);
        ConvertParticleIndexesToNonPeriodic(periodicIndexesMap, voronoiPlanes);
    }

    void VoronoiTesselationProvider::ConvertParticleIndexesToNonPeriodic(const vector<ParticleIndex>& periodicIndexesMap, vector<VoronoiPlane>* voronoiPlanes) const
    {
        // Convert all indexes to non-periodic
        for (vector<VoronoiPlane>::iterator it = voronoiPlanes->begin(); it != voronoiPlanes->end(); ++it)
        {
            VoronoiPlane& voronoiPlane = *it;
            voronoiPlane.particleIndex = periodicIndexesMap[voronoiPlane.particleIndex];
            voronoiPlane.neighborIndex = periodicIndexesMap[voronoiPlane.neighborIndex];
        }
    }

    void VoronoiTesselationProvider::ReadPeriodicVoronoiPlanes(string path, vector<VoronoiPlane>* voronoiPlanes) const
    {
        if (DIMENSIONS == 2)
        {
            throw NotImplementedException("TODO: check qhull in 2D, update this code.");
        }

        voronoiPlanes->clear();

        // Read
        ScopedFile<LogErrorHandler> file(path, FileOpenMode::Read);
        int voronoiPlanesCount;
        fscanf(file, "%d", &voronoiPlanesCount);

        for (int i = 0; i < voronoiPlanesCount; i++)
        {
            int dummy;
            VoronoiPlane plane;
            fscanf(file, "%d %d %d " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT " " FLOAT_FORMAT "\n",
                    &dummy,
                    &plane.particleIndex,
                    &plane.neighborIndex,
                    &plane.normal[Axis::X],
                    &plane.normal[Axis::Y],
                    &plane.normal[Axis::Z],
                    &plane.displacement);

            voronoiPlanes->push_back(plane);
        }
    }

    void VoronoiTesselationProvider::ReadPeriodicIndexesMap(string path, vector<ParticleIndex>* periodicIndexesMap) const
    {
        periodicIndexesMap->clear();
        ScopedFile<LogErrorHandler> file(path, FileOpenMode::Read);

        int mappedIndex;
        fscanf(file, "%d\n", &mappedIndex);

        bool fileIsOneBased = false;
        // periodicIndexesMap can be one-based after MATLAB codes operation
        if (mappedIndex == 1)
        {
            fileIsOneBased = true;
            mappedIndex--;
        }
        periodicIndexesMap->push_back(mappedIndex);

        while (true)
        {
            int mappedIndex;
            int result = fscanf(file, "%d\n", &mappedIndex);
            if (result == EOF)
            {
                break;
            }

            if (fileIsOneBased)
            {
                mappedIndex--;
            }
            periodicIndexesMap->push_back(mappedIndex);
        }
    }
}

