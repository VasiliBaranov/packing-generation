// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Utilities.h"
#include "../Headers/Exceptions.h"
#include <errno.h>
#include <climits>
#include <cstdlib>

#ifndef WINDOWS
#include <unistd.h>
#endif

using namespace std;

namespace Core
{
    // Don't want to use boost::threads, can't use C++11
    void Utilities::Sleep(int milliseconds)
    {
#ifdef WINDOWS
        Sleep(milliseconds);
#else
        usleep(milliseconds * 1000); // takes microseconds
#endif
    }

    bool Utilities::StringStartsWith(const string& stringToCheck, const string& possibleStart)
    {
        return stringToCheck.compare(0, possibleStart.length(), possibleStart) == 0;
    }

    bool Utilities::StringEndsWith(const string& stringToCheck, const string& possibleEnd)
    {
        return stringToCheck.compare(stringToCheck.length() - possibleEnd.length(), possibleEnd.length(), possibleEnd) == 0;
    }

    // Copied from http://stackoverflow.com/a/6154614/2029962 and slightly modified
    // No C++ streams! In C++11 there is finally a function stoi, but i can't use C++11
    int Utilities::ParseInt(const string& input, int base)
    {
        char* end;
        long result;
        errno = 0;
        result = strtol(input.c_str(), &end, base);
        if ((errno == ERANGE && result == LONG_MAX) || result > INT_MAX)
        {
            throw InvalidOperationException("Overflow");
        }
        if ((errno == ERANGE && result == LONG_MIN) || result < INT_MIN)
        {
            throw InvalidOperationException("Underflow");
        }
        if (input.size() == 0 || *end != '\0')
        {
            throw InvalidOperationException("Inconvertible");
        }
        return result;
    }
}

