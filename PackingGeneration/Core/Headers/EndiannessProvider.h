// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_EndiannessProvider_h
#define Core_Headers_EndiannessProvider_h

//#include "Constants.h"
#include "IEndiannessProvider.h"
#include "Macros.h"

namespace Core
{
    // Represents a class to determine the endianness of the system.
    class EndiannessProvider : public IEndiannessProvider
    {
    public:
        EndiannessProvider();

        OVERRIDE bool IsBigEndian() const;

    private:
        DISALLOW_COPY_AND_ASSIGN(EndiannessProvider);
    };
}


#endif /* Core_Headers_EndiannessProvider_h */
