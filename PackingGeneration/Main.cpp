// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "Execution/Headers/PackingTaskFactory.h"
#include "Core/Headers/Path.h"
#include "Core/Headers/MpiManager.h"
#include "Parallelism/Headers/TaskManager.h"
#include "Parallelism/Headers/RandomLoadBalancer.h"

using namespace std;
using namespace Execution;
using namespace Core;
using namespace Parallelism;

int main (int argc, char **argv)
{
    // Change buffering mode to store printf output correctly:
    // 1. when program is terminated with Load Sharing Facility, and buffer may not be flushed
    // 2. when output is redirected or added to a file (> log.txt or | tee log.txt) and an error occurs
    int bufferChangeResult = setvbuf(stdout, NULL, _IONBF, 0);
    if (bufferChangeResult != 0)
    {
        perror("Buffering mode could not be changed");
        return EXIT_FAILURE;
    }

    MpiManager::GetInstance()->Initialize(&argc, &argv);

    // Idea taken from http://www.open-mpi.org/faq/?category=debugging#serial-debuggers
    // Once you attach with a debugger, go up the function stack until you are in this block of code (you'll likely attach during the sleep())
    // then set the variable i to a nonzero value. With GDB, the syntax is:
    // (gdb) set var i = 7
    // Then set a breakpoint after your block of code and continue execution until the breakpoint is hit.
    // Now you have control of your live MPI application and use the full functionality of the debugger.
//#ifdef PARALLEL
//#ifdef DEBUG
//    int i = 0;
//    char hostname[256];
//    gethostname(hostname, sizeof(hostname));
//    printf("PID %d on %s ready for attach\n", getpid(), hostname);
//    fflush(stdout);
//    while (0 == i)
//    {
//        sleep(5);
//    }
//#endif
//#endif

    PackingTaskFactory packingTaskFactory(Path::GetCurrentWorkingFolder(), argc, argv);
    TaskManager taskManager(&packingTaskFactory);

    taskManager.SubmitTasks();

    MpiManager::GetInstance()->Finalize();

    return(0);
}

