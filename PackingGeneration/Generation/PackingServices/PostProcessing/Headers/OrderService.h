// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_OrderService_h
#define Generation_PackingServices_PostProcessing_Headers_OrderService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"

namespace PackingServices
{
    class OrderService : public BaseDistanceService
    {
    public:
        struct NeighborDirection
        {
            Model::ParticleIndex neighborIndex;
            Core::SpatialVector direction;
        };

        typedef std::vector<NeighborDirection> NeighborDirections;

        // See Jin, Makse (2010) A first-order phase transition defines the random close packing of hard spheres
        struct Order
        {
            Core::FLOAT_TYPE globalOrder;
            Core::FLOAT_TYPE localOrder;
        };

        // See Jin, Makse (2010) A first-order phase transition defines the random close packing of hard spheres
        // and Bargiel, Tory (2001) Packing fraction and measures of disorder of ultradense irregular packings of equal spheres. II. Transition from dense random packing
        struct LocalOrientationalDisorder
        {
            std::vector<std::string> referenceLatticeNames;
            std::vector<std::vector<Core::FLOAT_TYPE> > disordersPerParticle;
            std::vector<std::vector<Model::ParticleIndex> > closeNeighborsPerParticle;
        };

    private:
        struct ReferenceLattice
        {
            std::string name;
            int latticeVectorsCount;
            std::vector<Core::FLOAT_TYPE> sortedAngles;

            ReferenceLattice(std::string name, int latticeVectorsCount, const Core::FLOAT_TYPE uniqueAnglesInDegrees[], const int uniqueAnglesCounts[], int arraySize);
        };

    public:
        OrderService(MathService* mathService, INeighborProvider* neighborProvider);

        void SetParticles(const Model::Packing& particles);

        void FillPressures(const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<Core::FLOAT_TYPE>& energyPowers, std::vector<Core::FLOAT_TYPE>* pressures) const;

        void FillParticleDirections(std::vector<NeighborDirections>* particleDirections) const;

        Order GetOrder(int l) const; // l is the spherical harmonics index. See Song, Wang, Makse "First order phase transition"

        void FillLocalOrientationalDisorder(LocalOrientationalDisorder* localOrientationalDisorder) const;

    private:
        Core::FLOAT_TYPE GetGlobalOrder(const std::vector<NeighborDirections>& particleDirections, int l) const;

        Core::FLOAT_TYPE GetLocalOrder(const std::vector<NeighborDirections>& particleDirections, int l) const;

        Core::FLOAT_TYPE GetParticleOrder(const std::vector<NeighborDirection>& neighborDirections, int l) const;

        void FillLocalOrientationalDisorder(const std::vector<const ReferenceLattice*>& referenceLattices, LocalOrientationalDisorder* localOrientationalDisorder) const;

        DISALLOW_COPY_AND_ASSIGN(OrderService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_OrderService_h */

