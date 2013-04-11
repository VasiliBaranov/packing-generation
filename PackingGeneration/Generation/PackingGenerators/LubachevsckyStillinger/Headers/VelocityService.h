// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingGenerators_LubachevsckyStillinger_Headers_VelocityService_h
#define Generation_PackingGenerators_LubachevsckyStillinger_Headers_VelocityService_h

#include "Types.h"
#include "Core/Headers/Macros.h"

namespace PackingGenerators
{
    class VelocityService
    {
    public:
        Core::FLOAT_TYPE temperature;
        static const Core::FLOAT_TYPE mass;
        static const Core::FLOAT_TYPE boltzmannConstant;

    public:
        VelocityService();

        void FillVelocities(std::vector<MovingParticle>* particles) const;

        Core::FLOAT_TYPE GetActualKineticEnergy(const std::vector<MovingParticle>& particles) const;

        Core::FLOAT_TYPE GetActualTemperature(Core::FLOAT_TYPE kineticEnergy, int particlesCount) const;

        Core::FLOAT_TYPE GetExpectedKineticEnergy(int particlesCount) const;

        void RescaleVelocities(Core::FLOAT_TYPE currentTime, Core::FLOAT_TYPE actualKineticEnergy, std::vector<MovingParticle>* particles) const;

        void SynchronizeParticleWithCurrentTime(Core::FLOAT_TYPE currentTime, MovingParticle* particle) const;

    private:
        void FillInitialVelocity(Core::SpatialVector* velocity) const;

        DISALLOW_COPY_AND_ASSIGN(VelocityService);
    };
}

#endif /* Generation_PackingGenerators_LubachevsckyStillinger_Headers_VelocityService_h */

