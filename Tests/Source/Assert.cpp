// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Assert.h"

namespace Tests
{
    void Assert::IsTrue(bool value, std::string callerName)
    {
        if(!value)
        {
            std::cout << "Assertion failed for " << callerName << ".\n";
            exit(EXIT_FAILURE);
        }
    }

    void Assert::WaitForKey()
    {
        std::cout << "Press any key";
        char key;
        std::cin >> key;
    }
}
