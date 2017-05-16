// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Execution_Headers_PackingGenerationTask_h
#define Execution_Headers_PackingGenerationTask_h

#include <string>
#include "Core/Headers/StlUtilities.h"
#include "Parallelism/Headers/ITask.h"
#include "Generation/Model/Headers/Config.h"
namespace Generation { class GenerationManager; }
namespace PackingGenerators { class IPackingStep; }
namespace PackingServices { class GeometryService; }
namespace PackingServices { class INeighborProvider; }
namespace PackingServices { class DistanceService; }
namespace PackingServices { class MathService; }
namespace PackingServices { class IClosestPairProvider; }
namespace PackingServices { class IEnergyService; }
namespace PackingServices { class IPairPotential; }
namespace PackingServices { class PackingSerializer; }

namespace Execution
{
    // Implements a packing generation task for parallel execution.
    class PackingGenerationTask : public virtual Parallelism::ITask
    {
    private:
        int id;

    public:
        Model::ExecutionConfig userConfig;

    public:
        explicit PackingGenerationTask(std::string baseFolder, int id);

        ~PackingGenerationTask();

        // Gets the estimated difficulty of task execution, i.e. its weight in the tasks list.
        OVERRIDE Core::FLOAT_TYPE GetWeight() const;

        OVERRIDE int GetId() const;

        OVERRIDE void Execute();

    private:
        boost::shared_ptr<PackingGenerators::IPackingStep> CreatePackingStep(PackingServices::GeometryService* geometryService,
                PackingServices::INeighborProvider* neighborProvider,
                PackingServices::DistanceService* distanceService,
                PackingServices::MathService* mathService,
                PackingServices::IClosestPairProvider* closestPairProvider,
                PackingServices::IEnergyService* generationEnergyService,
                PackingServices::IEnergyService* contractionEnergyService,
                PackingServices::IPairPotential* bezrukovPotential,
                PackingServices::IPairPotential* impermeableAttractionPotential,
                PackingServices::IPairPotential* pairPotential,
                PackingServices::PackingSerializer* packingSerializer) const;

        void CallCorrectMethod(Generation::GenerationManager* generationManager) const;

        DISALLOW_COPY_AND_ASSIGN(PackingGenerationTask);
    };
}

#endif /* Execution_Headers_PackingGenerationTask_h */

