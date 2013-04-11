// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Constants_h
#define Core_Headers_Constants_h

#include "Types.h"
#include <limits>

//Use constants instead of #define, as they may lead to better compiler checks
namespace Core
{
    const FLOAT_TYPE PI = 3.14159265359;
    const FLOAT_TYPE EPSILON = std::numeric_limits<FLOAT_TYPE>::epsilon();
    const FLOAT_TYPE MAX_FLOAT_VALUE = std::numeric_limits<FLOAT_TYPE>::max();
}

#endif /* Core_Headers_Constants_h */
