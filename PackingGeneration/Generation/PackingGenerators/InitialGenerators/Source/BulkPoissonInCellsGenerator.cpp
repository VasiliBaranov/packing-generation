// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/BulkPoissonInCellsGenerator.h"
#include "Core/Headers/VectorUtilities.h"
#include "Generation/Model/Headers/Config.h"

using namespace Core;
using namespace Model;
using namespace std;

namespace PackingGenerators
{
    BulkPoissonInCellsGenerator::BulkPoissonInCellsGenerator()
    {

    }

    void BulkPoissonInCellsGenerator::SetContext(const ModellingContext& context)
    {
        this->context = &context;
        this->config = context.config;
    }

    void BulkPoissonInCellsGenerator::SetGenerationConfig(const GenerationConfig& generationConfig)
    {

    }

    void BulkPoissonInCellsGenerator::ArrangePacking(Packing* particles)
    {
        Packing& particlesRef = *particles;
        int cellsCount = config->particlesCount / PARTICLES_IN_CELL;
        int linearCellsCount = static_cast<int>(floor(pow(cellsCount, 1.0 / DIMENSIONS)));
        cellsCount = linearCellsCount * linearCellsCount * linearCellsCount;
        int particlesInCell = config->particlesCount / cellsCount;

        SpatialVector cellSize;
        VectorUtilities::DivideByValue(config->packingSize, static_cast<FLOAT_TYPE>(linearCellsCount), &cellSize);

        DiscreteSpatialVector cellCoordinates;
        VectorUtilities::InitializeWith(&cellCoordinates, 0.0);

        for (ParticleIndex cellIndex = 0; cellIndex < cellsCount; ++cellIndex)
        {
            SpatialVector cellCorner;
            VectorUtilities::Multiply(cellSize, cellCoordinates, &cellCorner);

            for (ParticleIndex i = 0; i < particlesInCell; ++i)
            {
                DomainParticle& particle = particlesRef[cellIndex * particlesInCell + i];
                VectorUtilities::InitializeWithRandoms(&particle.coordinates);
                VectorUtilities::Multiply(particle.coordinates, cellSize, &particle.coordinates);
                VectorUtilities::Add(particle.coordinates, cellCorner, &particle.coordinates);
            }

            cellCoordinates[Axis::X]++;
            if (cellCoordinates[Axis::X] == linearCellsCount)
            {
                cellCoordinates[Axis::X] = 0;
                cellCoordinates[Axis::Y]++;
                if (DIMENSIONS == 3 && cellCoordinates[Axis::Y] == linearCellsCount)
                {
                    cellCoordinates[Axis::Y] = 0;
                    cellCoordinates[Axis::Z]++;
                }
            }
        }

        // Spread possible left particles uniformly in the domain
        for (ParticleIndex i = particlesInCell * cellsCount; i < config->particlesCount; ++i)
        {
            DomainParticle& particle = particlesRef[i];
            VectorUtilities::InitializeWithRandoms(&particle.coordinates);
            VectorUtilities::Multiply(particle.coordinates, cellSize, &particle.coordinates);
        }
    }

    FLOAT_TYPE BulkPoissonInCellsGenerator::GetFinalInnerDiameterRatio() const
    {
        return 0.0;
    }

    BulkPoissonInCellsGenerator::~BulkPoissonInCellsGenerator()
    {

    }
}

