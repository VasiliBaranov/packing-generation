// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Types_h
#define Core_Headers_Types_h

#include <boost/array.hpp>

// TODO: Move to Core namespace and Constants.h
const int DIMENSIONS = 3;

// Use typedef instead of #define, as they may lead to better compiler checks
namespace Core
{
#ifdef SINGLE_PRECISION
    typedef float FLOAT_TYPE;
#else
    typedef double FLOAT_TYPE;
#endif

    typedef int INT_TYPE;

    struct Axis
    {
        enum Type
        {
            X = 0,
            Y = 1,
            Z = 2
        };
    };

    typedef boost::array<int, DIMENSIONS> DiscreteSpatialVector;
    typedef boost::array<FLOAT_TYPE, DIMENSIONS> SpatialVector;

    // NOTE: the types below are not so fundamental. May be move somewhere?
    template<class T>
    struct Nullable
    {
        T value;
        bool hasValue;
    };
}

#endif /* Core_Headers_Types_h */
