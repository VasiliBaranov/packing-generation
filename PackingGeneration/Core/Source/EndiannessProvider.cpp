// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/EndiannessProvider.h"
#include <stdint.h>

namespace Core
{
    EndiannessProvider::EndiannessProvider()
    {

    }

    //Wiki: on modern standard computers (i.e., implementing IEEE 754), one may in practice safely assume that the endianness is the same for floating point numbers as for integers.
    bool EndiannessProvider::IsBigEndian() const
    {
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return bint.c[0] == 1;
    }
}
