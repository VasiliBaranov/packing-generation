// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BulkPoissonGenerator.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    BulkPoissonGenerator::BulkPoissonGenerator()
    {

    }

    void BulkPoissonGenerator::SetContext(const ModellingContext& context)
    {
        this->context = &context;
        this->config = context.config;
    }

    void BulkPoissonGenerator::SetGenerationConfig(const GenerationConfig& generationConfig)
    {

    }

    void BulkPoissonGenerator::ArrangePacking(Packing* particles)
    {
        Packing& particlesRef = *particles;
        for (ParticleIndex i = 0; i < config->particlesCount; ++i)
        {
            VectorUtilities::InitializeWithRandoms(&particlesRef[i].coordinates);
            VectorUtilities::Multiply(config->packingSize, particlesRef[i].coordinates, &particlesRef[i].coordinates);
        }
    }

    FLOAT_TYPE BulkPoissonGenerator::GetFinalInnerDiameterRatio() const
    {
        return 0.0;
    }

    BulkPoissonGenerator::~BulkPoissonGenerator()
    {

    }
}

