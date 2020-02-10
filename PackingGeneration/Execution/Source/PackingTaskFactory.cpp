// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingTaskFactory.h"

#include <cstring>
#include "Core/Headers/Path.h"
#include "Core/Headers/Utilities.h"
#include "../Headers/PackingGenerationTask.h"
#include "Generation/Constants.h"

using namespace std;
using namespace Parallelism;
using namespace Model;
using namespace Core;
using namespace Generation;

namespace Execution
{
    PackingTaskFactory::PackingTaskFactory(string baseFolder, int argc, char **argv)
    {
        this->baseFolder = baseFolder;
        ParseConsoleArguments(argc, argv);
    }

    void PackingTaskFactory::ParseConsoleArguments(int argc, char **argv)
    {
        if (argc < 2)
        {
            return;
        }

        consoleArguments.reserve(argc - 1);
        for (int i = 1; i < argc; ++i)
        {
            consoleArguments.push_back(argv[i]);
        }
    }

    PackingTaskFactory::~PackingTaskFactory()
    {
    }

    void PackingTaskFactory::FillTasks(vector<boost::shared_ptr<ITask> >* tasks)
    {
        FillGenerationConfig();

        // Select all the config files in folders and subfolders
        vector<string> configFolders;
        FillConfigFolders(&configFolders);

        tasks->reserve(configFolders.size());

        // For each config create a task
        int id = 0;
        for (vector<string>::iterator it = configFolders.begin(); it < configFolders.end(); ++it)
        {
            // NOTE: don't use below
            // PackingGenerationTask* packingGenerationTask = reinterpret_cast<PackingGenerationTask*>(task.get());
            // It doesn't work under MCVS if virtual inheritance is used in
            // class PackingGenerationTask : public virtual Parallelism::ITask

            string currentFolder = *it;
            boost::shared_ptr<ITask> task(new PackingGenerationTask(currentFolder, id));
            boost::shared_ptr<PackingGenerationTask> packingGenerationTask = boost::dynamic_pointer_cast<PackingGenerationTask, ITask>(task);
            packingGenerationTask->userConfig.MergeWith(userConfig);
            tasks->push_back(task);
            id++;
        }
    }

    void PackingTaskFactory::FillConfigFolders(std::vector<std::string>* configFolders) const
    {
        string fileToCheck = (userConfig.generationConfig.executionMode == ExecutionMode::PackingGeneration) ? CONFIG_FILE_NAME : PACKING_FILE_NAME_NFO;
        Path::FillFoldersRecursively(baseFolder, fileToCheck, configFolders);
    }

    void PackingTaskFactory::FillGenerationConfig()
    {
        // Best program parameters
        GenerationConfig* generationConfig = &userConfig.generationConfig;

        generationConfig->executionMode = ExecutionMode::PackingGeneration;
        generationConfig->insertionRadiiCount = 1e7;
        generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerSimple;

        if (consoleArguments.size() > 0)
        {
            ParseGenerationArguments(generationConfig);
        }
    }

    //We should parse arguments in the factory, not in the task, as file list to be used for tasks creation may depend (and depends) on arguments.
    void PackingTaskFactory::ParseGenerationArguments(GenerationConfig* generationConfig)
    {
        if (consoleArguments[0] == "-radii")
        {
            generationConfig->executionMode = ExecutionMode::InsertionRadiiGeneration;
            if (consoleArguments.size() > 1)
            {
                generationConfig->insertionRadiiCount = Utilities::ParseInt(consoleArguments[1]);
            }
        }
        // Distances to surfaces
        else if (consoleArguments[0] == "-disttosurf")
        {
            generationConfig->executionMode = ExecutionMode::DistancesToClosestSurfacesCalculation;
            if (consoleArguments.size() > 1)
            {
                generationConfig->insertionRadiiCount = Utilities::ParseInt(consoleArguments[1]);
            }
        }
        else if (consoleArguments[0] == "-connumdist")
        {
            generationConfig->executionMode = ExecutionMode::ContactNumberDistributionCalculation;
        }
        else if (consoleArguments[0] == "-entropy")
        {
            generationConfig->executionMode = ExecutionMode::EntropyCalculation;
            if (consoleArguments.size() > 1)
            {
                generationConfig->insertionRadiiCount = Utilities::ParseInt(consoleArguments[1]);
            }
        }
        else if (consoleArguments[0] == "-directions")
        {
            generationConfig->executionMode = ExecutionMode::DirectionsCalculation;
        }
        else if (consoleArguments[0] == "-contraction")
        {
            generationConfig->executionMode = ExecutionMode::ContractionEnergyCalculation;
        }
        else if (consoleArguments[0] == "-order")
        {
            generationConfig->executionMode = ExecutionMode::OrderCalculation;
        }
        else if (consoleArguments[0] == "-hessian")
        {
            generationConfig->executionMode = ExecutionMode::HessianEigenvaluesCalculation;
        }
        else if (consoleArguments[0] == "-pressure")
        {
            generationConfig->executionMode = ExecutionMode::PressureCalculation;
        }
        // md means Molecular Dynamics
        else if (consoleArguments[0] == "-md")
        {
            generationConfig->executionMode = ExecutionMode::MolecularDynamicsCalculation;

            // NOTE: this is a dirty hack. TODO: add minEquilibrationCycles parameter
            generationConfig->insertionRadiiCount = -1;
            if (consoleArguments.size() > 1)
            {
                if (consoleArguments[1] == "-suppress")
                {
                    generationConfig->shouldSuppressCrystallization.value = true;
                    generationConfig->shouldSuppressCrystallization.hasValue = true;
                }
                else
                {
                    generationConfig->insertionRadiiCount = Utilities::ParseInt(consoleArguments[1]);
                }
            }
        }
        // ReMove rattlers
        else if (consoleArguments[0] == "-rm")
        {
            generationConfig->executionMode = ExecutionMode::RattlerRemoval;
        }
        // Pair Correlation
        else if (consoleArguments[0] == "-pc")
        {
            generationConfig->executionMode = ExecutionMode::PairCorrelationCalculation;
        }
        // Structure Factor
        else if (consoleArguments[0] == "-sf")
        {
            generationConfig->executionMode = ExecutionMode::StructureFactorCalculation;
            if (consoleArguments.size() > 1)
            {
                generationConfig->particlesToKeepForStructureFactor = Utilities::ParseInt(consoleArguments[1]);

                generationConfig->keepSmallParticlesForStructureFactor.hasValue = true;
                generationConfig->keepSmallParticlesForStructureFactor.value = true;

                if (generationConfig->particlesToKeepForStructureFactor < 0)
                {
                    generationConfig->particlesToKeepForStructureFactor = -generationConfig->particlesToKeepForStructureFactor;
                    generationConfig->keepSmallParticlesForStructureFactor.value = false;
                }
            }
            else
            {
                generationConfig->particlesToKeepForStructureFactor = -1;
                generationConfig->keepSmallParticlesForStructureFactor.hasValue = false;
            }
        }
        // LocalOrientationalDisorder
        else if (consoleArguments[0] == "-lod")
        {
            generationConfig->executionMode = ExecutionMode::LocalOrientationalDisorder;
        }
        // Molecular Dynamics Immediate
        else if (consoleArguments[0] == "-mdi")
        {
            generationConfig->executionMode = ExecutionMode::ImmediateMolecularDynamicsCalculation;
        }
        // NearestNeighborsCalculation. Calculates a nearest neighbor for each particle
        else if (consoleArguments[0] == "-nnc")
        {
            generationConfig->executionMode = ExecutionMode::NearestNeighborsCalculation;
        }
        // ActiveGeometryCalculation
        else if (consoleArguments[0] == "-agc")
        {
            generationConfig->executionMode = ExecutionMode::ActiveGeometryCalculation;
            if (consoleArguments.size() > 1)
            {
                // NOTE: a very dirty hack. Specify the expected number of active particles with insertionRadiiCount
                // TODO: fix
                generationConfig->insertionRadiiCount = Utilities::ParseInt(consoleArguments[1]);
            }
            else
            {
                generationConfig->insertionRadiiCount = -1;
            }
        }
        else if (consoleArguments[0] == "-permprob")
        {
            generationConfig->executionMode = ExecutionMode::SuccessfulPermutationProbability;
        }
        else
        {
            ParseAlgorithm(generationConfig);
        }
    }

    // TODO: use some option parsing library, e.g. boost::program_options
    void PackingTaskFactory::ParseAlgorithm(Model::GenerationConfig* generationConfig)
    {
        size_t optionsStartIndex = 0;

        // LubachevskyStillinger
        if (consoleArguments[0] == "-ls")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerSimple;
            optionsStartIndex++;
        }
        // GradualDensification
        else if (consoleArguments[0] == "-lsgd")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerGradualDensification;
            optionsStartIndex++;
        }
        // EquilibrationBetweenCompressions
        else if (consoleArguments[0] == "-lsebc")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerEquilibrationBetweenCompressions;
            optionsStartIndex++;
        }
        // ConstantPower
        else if (consoleArguments[0] == "-lscp")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerConstantPower;
            optionsStartIndex++;
        }
        // Biazzo
        else if (consoleArguments[0] == "-lsb")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::LubachevskyStillingerBiazzo;
            optionsStartIndex++;
        }
        // ForceBiasedAlgorithm
        else if (consoleArguments[0] == "-fba")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::ForceBiasedAlgorithm;
            optionsStartIndex++;
        }
        // OriginalJodreyTory
        else if (consoleArguments[0] == "-ojt")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::OriginalJodreyTory;
            optionsStartIndex++;
        }
        // KhirevichJodreyTory
        else if (consoleArguments[0] == "-kjt")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::KhirevichJodreyTory;
            optionsStartIndex++;
        }
        // ClosestJammingSearch. To avoid naming collision, use shortened "Zinchenko".
        else if (consoleArguments[0] == "-zin")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::ClosestJammingSearch;
            optionsStartIndex++;
        }
        // MonteCarlo algorithm (don't use -mc to avoid confusion with -md)
        else if (consoleArguments[0] == "-mca")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::MonteCarlo;
            optionsStartIndex++;
        }
        // ConjugateGradient
        else if (consoleArguments[0] == "-cja")
        {
            generationConfig->generationAlgorithm = PackingGenerationAlgorithm::ConjugateGradient;
            optionsStartIndex++;
        }

        if (consoleArguments.size() > optionsStartIndex && consoleArguments[optionsStartIndex] == "-suppress")
        {
            generationConfig->shouldSuppressCrystallization.value = true;
            generationConfig->shouldSuppressCrystallization.hasValue = true;
            optionsStartIndex++;
        }

        if (consoleArguments.size() > optionsStartIndex)
        {
            generationConfig->contractionRate = atof(consoleArguments[optionsStartIndex].c_str());
            optionsStartIndex++;
        }

//        if (consoleArguments.size() > optionsStartIndex)
//        {
//            generationConfig->finalContractionRate = atof(consoleArguments[optionsStartIndex].c_str());
//            optionsStartIndex++;
//        }
//
//        if (consoleArguments.size() > optionsStartIndex)
//        {
//            generationConfig->contractionRateDecreaseFactor = atof(consoleArguments[optionsStartIndex].c_str());
//        }
    }
}

