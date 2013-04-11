// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Execution_Headers_PackingTaskFactory_h
#define Execution_Headers_PackingTaskFactory_h

#include <string>
#include "Parallelism/Headers/ITaskFactory.h"
#include "Generation/Model/Headers/Config.h"

namespace Execution
{
    // Implements a packing generation task factory.
    class PackingTaskFactory : public virtual Parallelism::ITaskFactory
    {
    private:
        std::string baseFolder;
        std::vector<std::string> consoleArguments;

        Model::ExecutionConfig userConfig;

    public:
        PackingTaskFactory(std::string baseFolder, int argc, char **argv);

        ~PackingTaskFactory();

        OVERRIDE void FillTasks(std::vector<boost::shared_ptr<Parallelism::ITask> >* tasks);

    private:
        void ParseConsoleArguments(int argc, char **argv);

        void FillGenerationConfig();

        void ParseGenerationArguments(Model::GenerationConfig* generationConfig);

        void ParseAlgorithm(Model::GenerationConfig* generationConfig);

        void FillConfigFolders(std::vector<std::string>* configFolders) const;

        DISALLOW_COPY_AND_ASSIGN(PackingTaskFactory);
    };
}

#endif /* Execution_Headers_PackingTaskFactory_h */

