// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingTaskFactory.h"

#include <cstring>
#include "Core/Headers/Path.h"
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
        for (vector<string>::iterator it = configFolders.begin(); it < configFolders.end(); ++it)
        {
            string currentFolder = *it;
            // Can not create a PackingGenerationTask with new, set user config, then put it into shared_ptr, because if MergeWith throws an exception, packingGenerationTask will not be deleted.
            boost::shared_ptr<ITask> task(new PackingGenerationTask(currentFolder));
            PackingGenerationTask* packingGenerationTask = reinterpret_cast<PackingGenerationTask*>(task.get());
            packingGenerationTask->userConfig.MergeWith(userConfig);
            tasks->push_back(task);
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
        generationConfig->maxRunsCount = 1;
        generationConfig->stopOnTheoreticalDensity = false;
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
                generationConfig->insertionRadiiCount = atoi(consoleArguments[1].c_str());
            }
        }
        else if (consoleArguments[0] == "-entropy")
        {
            generationConfig->executionMode = ExecutionMode::EntropyCalculation;
            if (consoleArguments.size() > 1)
            {
                generationConfig->insertionRadiiCount = atoi(consoleArguments[1].c_str());
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

        if (consoleArguments.size() > optionsStartIndex)
        {
            generationConfig->contractionRate = atof(consoleArguments[optionsStartIndex].c_str());
        }
        if (consoleArguments.size() > optionsStartIndex + 1)
        {
            generationConfig->maxIterations = atoi(consoleArguments[optionsStartIndex + 1].c_str());
        }
        if (consoleArguments.size() > optionsStartIndex + 2)
        {
            generationConfig->maxRunsCount = atoi(consoleArguments[optionsStartIndex + 2].c_str());
        }
        if (consoleArguments.size() > optionsStartIndex + 3)
        {
            generationConfig->minRunsCount = atoi(consoleArguments[optionsStartIndex + 3].c_str());
        }
        if (consoleArguments.size() > (optionsStartIndex + 4) && consoleArguments[optionsStartIndex + 4] == "-stop")
        {
            generationConfig->stopOnTheoreticalDensity = true;
        }
    }
}

