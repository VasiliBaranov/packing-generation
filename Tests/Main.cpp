// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "Headers/TestRunner.h"

using namespace Tests;

// Do not use CppUnit / boost::tests / googletests as this would make compiling by other members and on supercomputers much harder.
// Do not put tests into a separate project and exe, as this would require extracting all the testable logic into a separate static library;
// Eclipse has a drawback when generating makefiles for projects with dependencies: a target for building a dependency is called by the full Windows path to the dependency project,
// i.e. contains semicolon, which Unix make won't understand (see http://stackoverflow.com/questions/2100448/multiple-target-patterns-makefile-error).

// To run and debug tests
// 1. Add another build configuration in Eclipse (Test), go to Project properties->Settings->Cygwin C++ compiler->Preprocessor->Add defined symbol "TEST"
// 2. Go to Debug Configurations (in the Debug dropdown), add new configuration, specify Test/PackingGeneration.exe as executable
int main (int argc, char **argv)
{
    TestRunner::RunTests();

    return 0;
}

