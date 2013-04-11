// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h
#define Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h

#include "Core/Headers/Macros.h"
#include "Generation/Model/Headers/Types.h"
#include "Generation/PackingServices/Headers/IContextDependentService.h"
namespace PackingServices { class GeometryService; }
namespace PackingServices { class DistanceService; }
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

    public:
        InsertionRadiiGenerator(DistanceService* distanceProvider, GeometryService* geometryService);

        OVERRIDE void SetContext(const Model::ModellingContext& context);

        void FillInsertionRadii(const Model::Packing& particles, int insertionRadiiCount, std::vector<Core::FLOAT_TYPE>* insertionRadii) const;

        Core::FLOAT_TYPE CalculateEntropy(const Model::Packing& particles, int insertionRadiiCount) const;

        virtual ~InsertionRadiiGenerator();

    private:
        Core::FLOAT_TYPE GetLargePoresDensity(const Model::Packing& particles, int insertionRadiiCount, Core::FLOAT_TYPE minPoreRadius) const;

        Core::FLOAT_TYPE CalculateLocalEntropy(const Core::SpatialVector& point, Core::FLOAT_TYPE meanDiameter) const;

        DISALLOW_COPY_AND_ASSIGN(InsertionRadiiGenerator);
    };
}

#endif /* Generation_PackingServices_PostProcessing_Headers_InsertionRadiiGenerator_h */

