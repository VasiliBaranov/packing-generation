// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_PressureService_h
#define Generation_PackingServices_PostProcessing_Headers_PressureService_h

#include "Generation/PackingServices/DistanceServices/Headers/BaseDistanceService.h"

namespace PackingServices
{
    // Class capable of computing pressure tensor, bulk modulus, shear modulus.
    // See O'�Hern et al (2003) The epitome of disorder.
    class PressureService : public BaseDistanceService
    {
    public:
        PressureService(MathService* mathService, INeighborProvider* neighborProvider);

        void SetParticles(const Model::Packing& particles);

        void FillPressures(const std::vector<Core::FLOAT_TYPE>& contractionRatios, const std::vector<Core::FLOAT_TYPE>& energyPowers, std::vector<Core::FLOAT_TYPE>* pressures) const;

        Core::FLOAT_TYPE GetBulkModulus() const;

    private:
        Core::FLOAT_TYPE GetPressure(Core::FLOAT_TYPE contractionRatio, Core::FLOAT_TYPE energyPower) const;

        void FillPressureTensor(Core::FLOAT_TYPE contractionRatio, Core::FLOAT_TYPE energyPower, Core::FLOAT_TYPE pressureTensor[DIMENSIONS][DIMENSIONS]) const;

        void UpdatePressureTensor(Core::FLOAT_TYPE contractionRatio, Core::FLOAT_TYPE energyPower, const Model::DomainParticle& particle, const Model::DomainParticle& neighbor, Core::FLOAT_TYPE pressureTensor[DIMENSIONS][DIMENSIONS]) const;

        Core::FLOAT_TYPE GetPressureEntry(const Model::DomainParticle& particle, const Model::DomainParticle& neighbor,
                int firstPressureDimension, int secondPressureDimension,
                const Core::SpatialVector& direction, Core::FLOAT_TYPE distance, Core::FLOAT_TYPE energyPower) const;

        DISALLOW_COPY_AND_ASSIGN(PressureService);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_PressureService_h */

