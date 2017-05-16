// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/GeometryCollisionService.h"

#include "Core/Headers/VectorUtilities.h"
#include "Core/Headers/Constants.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingServices
{
    GeometryCollisionService::GeometryCollisionService()
    {

    }

    // See http://en.wikipedia.org/wiki/Line-plane_intersection.
    // The dot products are optimized to account for perpendicular walls.
    FLOAT_TYPE GeometryCollisionService::GetPlaneIntersectionTime(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const SimplePlane& plane) const
    {
//        FLOAT_TYPE velocityProjection = velocity[plane.perpendicularAxis];
//        if (velocityProjection == 0.0)
//        {
//            return -1;
//        }
//
//        FLOAT_TYPE particleCoordinateOnAxis = point[plane.perpendicularAxis];
//        FLOAT_TYPE differenceProjection = plane.coordinateOnAxis - particleCoordinateOnAxis;
//        FLOAT_TYPE transferTime = differenceProjection / velocityProjection;
//
//        return transferTime;


        // Assume that particles can be outside the box only due to finite precision errors after periodic shifts.
        // We search only for intersections which make particle leave the cube.
        // I.e., particle velocity shall have a component in the direction of the outer normal of the plane (i.e., non-zero, of the same sign).

        FLOAT_TYPE particleCoordinateOnAxis = point[plane.perpendicularAxis];
        FLOAT_TYPE distanceToWallProjectionOnAxis = plane.coordinateOnAxis - particleCoordinateOnAxis;
        FLOAT_TYPE velocityProjectionOnAxis = velocity[plane.perpendicularAxis];

        FLOAT_TYPE distanceToWallProjectionOnOuterNormal = distanceToWallProjectionOnAxis * plane.outerNormalDirection;

        // Particle is on a plane or outside the box
        if (distanceToWallProjectionOnOuterNormal <= 0)
        {
            FLOAT_TYPE velocityProjectionOnOuterNormal = velocityProjectionOnAxis * plane.outerNormalDirection;

            // Particle is outside the box
            if (distanceToWallProjectionOnOuterNormal < 0)
            {
                // Should not return every time when the particle is outside the box.
                // Imagine the case:
                // 1. the particle is inside the box, is directed outside the box
                // 2. it crosses the boundary, is periodically shifted
                // 3. due to roundoff errors it may be slightly outside the box, but its velocity direction is now inside the box
                // 4. if we once again try shift it periodically, we may once again appear outside the box, and enter the infinite loop.
                // This loop may still happen for immobile particles. TODO: think, maybe to update the code?
                // NOTE: currently the wall to which the particle was shifted is excluded from searching the intersections,
                // so even if we enter the infinite loop for immobile particles, collisions will be processed,
                // and the particle will finally obtain a non-zero velocity through collisions

                // Moving outside the box or immobile - return 0; moving inside the box - return -1
                bool movingOutsideBoxOrImmobile = velocityProjectionOnOuterNormal >= 0;
                FLOAT_TYPE collisionTime = movingOutsideBoxOrImmobile ? 0 : -1;
                return collisionTime;
            }
            // Particle is on a plane
            else
            {
                // Moving outside the box - return 0; moving inside the box or immobile - return -1
                bool movingOutsideBox = velocityProjectionOnOuterNormal > 0;
                FLOAT_TYPE collisionTime = movingOutsideBox ? 0 : -1;
                return collisionTime;
            }
        }
        // Inside the box
        else
        {
            // Velocity projection zero - return -1
            if (velocityProjectionOnAxis == 0.0)
            {
                return -1;
            }
            // Non-zero velocity - return computation result
            else
            {
                FLOAT_TYPE transferTime = distanceToWallProjectionOnAxis / velocityProjectionOnAxis;
                return transferTime;
            }
        }
    }

    FLOAT_TYPE GeometryCollisionService::GetPlaneIntersectionTime(const SpatialVector& point, const SpatialVector& velocity, const Model::Plane& plane) const
    {
        FLOAT_TYPE velocityNormalProduct = VectorUtilities::GetDotProduct(plane.normal, velocity);
        if (velocityNormalProduct == 0.0)
        {
            return -1;
        }

        FLOAT_TYPE differenceNormalProduct = plane.displacement + VectorUtilities::GetDotProduct(plane.normal, point);
        FLOAT_TYPE transferTime = - differenceNormalProduct / velocityNormalProduct;

        return transferTime;
    }

    // See http://mathworld.wolfram.com/Point-PlaneDistance.html
    FLOAT_TYPE GeometryCollisionService::GetDistance(const SpatialVector& point, const Plane& plane) const
    {
        // Equivalent to calling GetPlaneIntersectionTime for velocity = - plane->normal
        return VectorUtilities::GetDotProduct(point, plane.normal) + plane.displacement;
    }

    FLOAT_TYPE GeometryCollisionService::GetSphereIntersectionTime(const Core::SpatialVector& point, const Core::SpatialVector& velocity, const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const
    {
        FLOAT_TYPE velocityLength = VectorUtilities::GetLength(velocity);
        if (velocityLength == 0.0)
        {
            return -1;
        }

        SpatialVector direction;
        VectorUtilities::DivideByValue(velocity, velocityLength, &direction);

        SpatialVector shiftedSphereCenter;
        VectorUtilities::Subtract(sphereCenter, point, &shiftedSphereCenter);

        FLOAT_TYPE dotProduct = VectorUtilities::GetDotProduct(direction, shiftedSphereCenter);
        FLOAT_TYPE sphereCenterSquare = VectorUtilities::GetSelfDotProduct(shiftedSphereCenter);
        FLOAT_TYPE discriminant = dotProduct * dotProduct - sphereCenterSquare +  sphereRadius * sphereRadius;

        if (discriminant < 0.0)
        {
            return -1;
        }

        FLOAT_TYPE discriminantRoot = sqrt(discriminant);
        FLOAT_TYPE distance = dotProduct - discriminantRoot;
        if (distance < 0.0)
        {
            distance = dotProduct + discriminantRoot;
        }

        return distance / velocityLength;
    }

    void GeometryCollisionService::FindIntersection(const SpatialVector& point, const SpatialVector& velocity, const CubicBox& box, int wallIndexToExclude, FLOAT_TYPE* intersectionTime, int* intersectionWallIndex) const
    {
        Core::FLOAT_TYPE& intersectionTimeRef = *intersectionTime;
        intersectionTimeRef = MAX_FLOAT_VALUE;
        int& intersectionWallIndexRef = *intersectionWallIndex;
        intersectionWallIndexRef = -1;
        for (int i = 0; i < CubicBox::wallsCount; ++i)
        {
            if (i == wallIndexToExclude)
            {
                continue;
            }

            Core::FLOAT_TYPE currentIntersectionTime = GetPlaneIntersectionTime(point, velocity, box.walls[i]);

            // A particle may cross a wall and also a continuation of another wall plane outside the box, that's why we can't return the first encountered transfer time.
            if (currentIntersectionTime >= 0.0 && currentIntersectionTime < intersectionTimeRef)
//            if (currentIntersectionTime > 0.0 && currentIntersectionTime < intersectionTimeRef)
            {
                intersectionTimeRef = currentIntersectionTime;
                intersectionWallIndexRef = i;
            }
        }
    }

    // TODO: remove code duplication with FindIntersection for CubicBox
    void GeometryCollisionService::FindIntersection(const Core::SpatialVector& point, const SpatialVector& velocity, const Polytope& box, int wallIndexToExclude, FLOAT_TYPE* intersectionTime, int* intersectionWallIndex) const
    {
        Core::FLOAT_TYPE& intersectionTimeRef = *intersectionTime;
        intersectionTimeRef = MAX_FLOAT_VALUE;
        int& intersectionWallIndexRef = *intersectionWallIndex;
        intersectionWallIndexRef = -1;
        int index = 0;
        for (vector<Plane>::const_iterator it = box.planes.begin(); it != box.planes.end(); ++it)
        {
            if (index == wallIndexToExclude)
            {
                continue;
            }

            const Plane& plane = *it;
            FLOAT_TYPE currentIntersectionTime = GetPlaneIntersectionTime(point, velocity, plane);

            // A particle may cross a wall and also a continuation of another wall plane outside the box, that's why we can't return the first encountered transfer time.
            if (currentIntersectionTime > 0.0 && currentIntersectionTime < intersectionTimeRef)
            {
                intersectionTimeRef = currentIntersectionTime;
                intersectionWallIndexRef = index;
            }
            index++;
        }
    }
}

