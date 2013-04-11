// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTesselationProvider_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTesselationProvider_h

#include "Core/Headers/Macros.h"
#include "Types.h"
namespace PackingServices { class MathService; }
namespace PackingServices { class GeometryCollisionService; }

namespace PackingGenerators
{
    class VoronoiTesselationProvider
    {
    private:
        PackingServices::GeometryCollisionService* geometryCollisionService;
        PackingServices::MathService* mathService;

        std::string baseFolder;
        const Model::Packing* particles;
        Model::ParticleIndex particlesCount;

    public:
        VoronoiTesselationProvider(PackingServices::GeometryCollisionService* geometryCollisionService,
                PackingServices::MathService* mathService,
                std::string baseFolder,
                const Model::Packing* particles,
                Model::ParticleIndex particlesCount);

        void FillVoronoiTesselation(std::vector<VoronoiPolytope>* voronoiTesselation) const;

        ~VoronoiTesselationProvider() {};

    private:
        void ReadPeriodicIndexesMap(std::string path, std::vector<Model::ParticleIndex>* periodicIndexesMap) const;

        void ReadVoronoiPlanes(std::string path, const std::vector<Model::ParticleIndex>& periodicIndexesMap, std::vector<VoronoiPlane>* voronoiPlanes) const;

        void ReadPeriodicVoronoiPlanes(std::string path, std::vector<VoronoiPlane>* voronoiPlanes) const;

        void ConvertParticleIndexesToNonPeriodic(const std::vector<Model::ParticleIndex>& periodicIndexesMap, std::vector<VoronoiPlane>* voronoiPlanes) const;

        void FillPlanesPerParticle(const std::vector<VoronoiPlane>& voronoiPlanes, std::vector<std::vector<VoronoiPlane> >* voronoiPlanesPerParticle) const;

        void FillNonUniquePlanesPerParticle(const std::vector<VoronoiPlane>& voronoiPlanes, std::vector<std::vector<VoronoiPlane> >* voronoiPlanesPerParticle) const;

        void RemoveDuplicatePlanesPerParticle(std::vector<std::vector<VoronoiPlane> >* voronoiPlanesPerParticle) const;

        void FillVoronoiTesselation(const std::vector<std::vector<VoronoiPlane> >& voronoiPlanesPerParticle, std::vector<VoronoiPolytope>* voronoiTesselation) const;

        void SetInscribedSpheres(const Model::Packing& particles, std::vector<VoronoiPolytope>* voronoiTesselation) const;

        void SetInscribedSphere(const Model::DomainParticle& particle, VoronoiPolytope* polytope) const;

        DISALLOW_COPY_AND_ASSIGN(VoronoiTesselationProvider);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_VoronoiTesselationProvider_h */

