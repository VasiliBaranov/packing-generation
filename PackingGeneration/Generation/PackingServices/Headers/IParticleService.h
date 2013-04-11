// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_IParticleService_h
#define Generation_PackingServices_Headers_IParticleService_h

#include "Generation/Model/Headers/Types.h"

namespace PackingServices
{
    // Defines methods for any service over particles packing, which supports particle displacement.
    class IParticleService
    {
    public:
        virtual void SetParticles(const Model::Packing& particles) = 0;

        //Use indexes, not pointers to particles, as otherwise we would implicitly assume that these pointers point to the "particles" array field (which is a hidden dependency and caused bugs already).
        virtual void StartMove(Model::ParticleIndex particleIndex) = 0;

        virtual void EndMove() = 0;

        virtual ~IParticleService(){ };
    };
}

#endif /* Generation_PackingServices_Headers_IParticleService_h */

