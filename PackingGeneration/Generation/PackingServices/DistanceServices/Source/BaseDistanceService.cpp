// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BaseDistanceService.h"

#include "Generation/PackingServices/Headers/MathService.h"
#include "../Headers/INeighborProvider.h"
#include "Generation/Model/Headers/Config.h"

using namespace Model;

namespace PackingServices
{
    BaseDistanceService::BaseDistanceService(MathService* mathService, INeighborProvider* neighborProvider)
    {
        this->mathService = mathService;
        this->neighborProvider = neighborProvider;
    }

    void BaseDistanceService::SetContext(const ModellingContext& context)
    {
        config = context.config;
        geometry = context.geometry;
        mathService->SetContext(context);
        neighborProvider->SetContext(context);
    }

    INeighborProvider* BaseDistanceService::GetNeighborProvider() const
    {
        return neighborProvider;
    }
}

