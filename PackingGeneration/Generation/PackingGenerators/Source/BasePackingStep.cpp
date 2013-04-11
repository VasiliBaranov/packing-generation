// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BasePackingStep.h"
#include "Generation/PackingServices/Headers/MathService.h"
#include "Generation/PackingServices/Headers/GeometryService.h"
#include "Generation/PackingServices/DistanceServices/Headers/INeighborProvider.h"
#include "Generation/PackingServices/DistanceServices/Headers/IClosestPairProvider.h"
#include "Generation/Geometries/Headers/IGeometry.h"
#include "Generation/Model/Headers/Config.h"
#include "Generation/Constants.h"

using namespace std;
using namespace Geometries;
using namespace PackingServices;
using namespace Core;
using namespace Model;
using namespace Generation;

namespace PackingGenerators
{
    BasePackingStep::BasePackingStep(GeometryService* geometryService,
            INeighborProvider* neighborProvider,
            IClosestPairProvider* closestPairProvider,
            MathService* mathService)
    {
        this->geometryService = geometryService;
        this->neighborProvider = neighborProvider;
        this->closestPairProvider = closestPairProvider;
        this->mathService = mathService;

        outerDiameterRatio = 1.0;
        innerDiameterRatio = 1.0;

        isOuterDiameterChanging = false;
        canOvercomeTheoreticalDensity = false;
    }

    BasePackingStep::~BasePackingStep()
    {

    }

    void BasePackingStep::SetGenerationConfig(const GenerationConfig& generationConfig)
    {
        this->generationConfig = &generationConfig;
    }

    void BasePackingStep::SetContext(const ModellingContext& context)
    {
        this->context = &context;
        this->config = context.config;
        this->geometry = context.geometry;

        if (geometryService != NULL) geometryService->SetContext(context);
        if (neighborProvider != NULL) neighborProvider->SetContext(context);
        if (closestPairProvider != NULL) closestPairProvider->SetContext(context);
        if (mathService != NULL) mathService->SetContext(context);
    }

    void BasePackingStep::SetParticles(Packing* particles)
    {
        this->particles = particles;
        if (neighborProvider != NULL) neighborProvider->SetParticles(*particles);
        if (closestPairProvider != NULL) closestPairProvider->SetParticles(*particles);

        particlesVolume = geometryService->GetParticlesVolume(*particles);
        totalVolume = geometry->GetTotalVolume();
        theoreticalPorosity = CalculateTheoreticalPorosity();
    }

    FLOAT_TYPE BasePackingStep::GetInnerDiameterRatio() const
    {
        return innerDiameterRatio;
    }

    FLOAT_TYPE BasePackingStep::GetOuterDiameterRatio() const
    {
        return outerDiameterRatio;
    }


    bool BasePackingStep::IsOuterDiameterChanging() const
    {
        return isOuterDiameterChanging;
    }

    bool BasePackingStep::CanOvercomeTheoreticalDensity() const
    {
        return canOvercomeTheoreticalDensity;
    }

    bool BasePackingStep::ShouldContinue() const
    {
        const FLOAT_TYPE MIN_DISTANCE = (2.0 - TOLERANCE);

        // Don't write through logical expressions, as will be too complicated
        if (isOuterDiameterChanging)
        {
            if (outerDiameterRatio <= innerDiameterRatio)
            {
                return false;
            }
            else
            {
                if (!generationConfig->stopOnTheoreticalDensity)
                {
                    return true;
                }
//                else
//                {
//                    go to CanOvercomeTheoreticalDensity() block
//                }
            }
        }

        // Get here if outer diameter is not changing or if we should stop on theoretical density
        if (canOvercomeTheoreticalDensity)
        {
            return innerDiameterRatio < 1.0;
        }
        else
        {
            return innerDiameterRatio < MIN_DISTANCE;
        }
    }

    FLOAT_TYPE BasePackingStep::CalculateTheoreticalPorosity() const
    {
        return CalculateCurrentPorosity(1.0);
    }

    FLOAT_TYPE BasePackingStep::CalculateCurrentPorosity(FLOAT_TYPE diameterRatio) const
    {
        FLOAT_TYPE calculatedPorosity = 1.0 - (particlesVolume * diameterRatio * diameterRatio * diameterRatio) / totalVolume;
        return calculatedPorosity;
    }
}

