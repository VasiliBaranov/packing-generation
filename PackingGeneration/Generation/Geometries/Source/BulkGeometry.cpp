// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BulkGeometry.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;
using namespace Core;

namespace Geometries
{
    BulkGeometry::BulkGeometry(const Model::SystemConfig& config) : BaseGeometry(config)
    {

    }

    BulkGeometry::~BulkGeometry()
    {

    }

    FLOAT_TYPE BulkGeometry::GetTotalVolume() const
    {
        return VectorUtilities::GetProduct(config->packingSize);
    }

    void BulkGeometry::EnsurePeriodicConditions(Model::DomainParticle* firstParticle, Model::DomainParticle* secondParticle) const
    {
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            EnsureVerticalPeriodicity(&firstParticle->coordinates, &secondParticle->coordinates, i);
        }
    }

    void BulkGeometry::EnsureBoundaries(const Model::DomainParticle& initialParticle, Model::DomainParticle* movedParticle, FLOAT_TYPE minNormalizedDistance) const
    {
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            EnsureVerticalPeriodicityAfterRepulsion(&movedParticle->coordinates, i);
        }
    }

    bool BulkGeometry::IsSphereInside(const Core::SpatialVector& sphereCenter, Core::FLOAT_TYPE sphereRadius) const
    {
        return true;
    }
}

