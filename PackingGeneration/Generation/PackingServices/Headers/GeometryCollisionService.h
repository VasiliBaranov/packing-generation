// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_GeometryCollisionService_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_GeometryCollisionService_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"

namespace PackingServices
{
    //NOTE: this class introduces time into the domain model (which is bad due to the Occam's razor;
    // clients of this service are also forced to use the time entity, e.g. INeighborProvider).
    // Indeed, it is possible to avoid usage of time, and name the methods "GetDistanceToPlane/Sphere".
    // But then it restricts class flexibility (e.g. it's impossible to use some optimizations, see GetPlaneIntersectionTime).
    // NOTE: this class doesn't impose periodic boundary conditions! This is done to allow performance optimizations
    // (esp. for Voronoi tesselation intersections search: it's easier to store some of the periodic images of Voronoi planes, than to analyze 27 periodic reflections for each plane).
    // If you start imposing periodic conditions in this class, don't forget to:
    // 1. leave just 3 planes in a Model::CubicBox
    // 2. in WallTransferEventProcessor move particle periodically not by the wall outer normal, but by particle velocity direction
    class GeometryCollisionService
    {
    public:
        GeometryCollisionService();

        // This method finds time of intersection with periodic box boundaries.
        // Returns a negative number if the boundary is never crossed.
        // It actually should be polymorphic by geometry. TODO: add a similar method to IGeometry, use this service in geometries.
        Core::FLOAT_TYPE GetPlaneIntersectionTime(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Model::SimplePlane& plane) const;

        Core::FLOAT_TYPE GetPlaneIntersectionTime(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Model::Plane& plane) const;

        Core::FLOAT_TYPE GetDistance(const Core::SpatialVector& point, const Model::Plane& plane) const;

        // Returns a negative number if the boundary is never crossed.
        Core::FLOAT_TYPE GetSphereIntersectionTime(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const;

        // intersectionTime is negative if there is no intersection.
        void FindIntersection(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Model::CubicBox& box, int wallIndexToExclude, Core::FLOAT_TYPE* intersectionTime, int* intersectionWallIndex) const;

        // intersectionTime is negative if there is no intersection.
        void FindIntersection(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Model::Polytope& box, int wallIndexToExclude, Core::FLOAT_TYPE* intersectionTime, int* intersectionWallIndex) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(GeometryCollisionService);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_GeometryCollisionService_h */

