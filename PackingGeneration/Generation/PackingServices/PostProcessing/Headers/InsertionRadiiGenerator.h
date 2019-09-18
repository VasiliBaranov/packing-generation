// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h
#define Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
#include "Generation/PackingServices/EnergyServices/Headers/IEnergyService.h"

namespace PackingServices { class GeometryService; }
namespace PackingServices { class DistanceService; }
namespace PackingServices { class PackingSerializer; }
namespace PackingServices { class IPairPotential; }
namespace Geometries { class IGeometry; }
namespace Model { class SystemConfig; }
namespace Model { class ModellingContext; }

namespace PackingServices
{
    // Represents a class to generate packing insertion radii.
    class InsertionRadiiGenerator : public virtual IContextDependentService
    {
    private:
        DistanceService* distanceProvider;
        GeometryService* geometryService;
        const Geometries::IGeometry* geometry;
        const Model::SystemConfig* config;
        const Model::ModellingContext* context;

    public:
        InsertionRadiiGenerator(DistanceService* distanceProvider, GeometryService* geometryService);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        void FillInsertionRadii(const Model::Packing& particles, int insertionRadiiCount, std::vector<Core::FLOAT_TYPE>* insertionRadii) const;

        Core::FLOAT_TYPE CalculateEntropy(const Model::Packing& particles, int insertionRadiiCount) const;

        void FillDistancesToSurfaces(const Model::Packing& particles, int samplePointsCount, const std::vector<int>& sortedSurfaceIndexes, std::string distancesFolderPath, const PackingSerializer& packingSerializer) const;

        // These functions will be used primarily for calculation of entropy with fixed coordination number
        Core::FLOAT_TYPE GetContractionRateForCoordinationNumber(IEnergyService* energyService, Core::FLOAT_TYPE expectedAverageCoordinationNumber);

        Core::FLOAT_TYPE GetContactNumberDistribution(const Model::Packing& particles, 
            IEnergyService* energyService, 
            Core::FLOAT_TYPE contractionRate, 
            std::vector<int>* neighborCounts, 
            std::vector<int>* neighborCountFrequencies,
            std::vector<std::vector<int>>* touchingParticleIndexes) const;

        void FillNormalizedContactingNeighborDistances(const Model::Packing& particles,
            const std::vector<std::vector<int>>& touchingParticleIndexes,
            std::vector<Core::FLOAT_TYPE>* normalizedContactingNeighborDistances) const;

        Core::FLOAT_TYPE GetSuccessfulPermutationProbability(Model::Packing* particles, int maxAttemptsCount) const;

        virtual ~InsertionRadiiGenerator();

    private:
        void FillRandomPoint(Core::SpatialVector* point) const;

        DISALLOW_COPY_AND_ASSIGN(InsertionRadiiGenerator);

        class GetContractionEnergyFunctor
        {
        private:
            const std::vector<const IPairPotential*>& potentials;
            IEnergyService* energyService;

        public:
            GetContractionEnergyFunctor(const std::vector<const IPairPotential*>& potentialsParam,
                    IEnergyService* energyServiceParam) :
                        potentials(potentialsParam),
                        energyService(energyServiceParam)
            {
            };

            Core::FLOAT_TYPE operator()(Core::FLOAT_TYPE negativeContractionRatio)
            {
                std::vector<Core::FLOAT_TYPE> contractionRatios(1, -negativeContractionRatio);
                IEnergyService::EnergiesResult result = energyService->GetContractionEnergies(contractionRatios, potentials);
                Core::FLOAT_TYPE coordinationNumber = result.contractionEnergies[0] / result.nonRattlersCounts[0];
                return coordinationNumber;
            };
        };
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h */

