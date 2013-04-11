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

        struct Order
        {
            Core::FLOAT_TYPE globalOrder;
            Core::FLOAT_TYPE localOrder;
        };

    public:
        OrderService(MathService* mathService, INeighborProvider* neighborProvider);

        void SetParticles(const Model::Packing& particles);

        void FillPressures(const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<Core::FLOAT_TYPE>& energyPowers, std::vector<Core::FLOAT_TYPE>* pressures) const;

        void FillParticleDirections(std::vector<NeighborDirections>* particleDirections) const;

        Order GetOrder(int l) const; // l is the spherical harmonics index. See Song, Wang, Makse "First order phase transition"

    private:
        Core::FLOAT_TYPE GetGlobalOrder(const std::vector<NeighborDirections>& particleDirections, int l) const;

        Core::FLOAT_TYPE GetLocalOrder(const std::vector<NeighborDirections>& particleDirections, int l) const;

        Core::FLOAT_TYPE GetParticleOrder(const std::vector<NeighborDirection>& neighborDirections, int l) const;

        DISALLOW_COPY_AND_ASSIGN(OrderService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_OrderService_h */

