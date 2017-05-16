// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Types_h
#define Core_Headers_Types_h

#include <boost/array.hpp>

// TODO: Move to Core namespace and Constants.h
// TWO_DIMENSIONAL define may be used only here and in Tests to choose between 2D or 3D versions.
#ifdef TWO_DIMENSIONAL
const int DIMENSIONS = 2;
#else
const int DIMENSIONS = 3;
#endif

#ifdef TWO_DIMENSIONAL
#define REMOVE_LAST_DIMENSION_IF_NEEDED(X, Y, Z) {{(X), (Y)}}
#else
#define REMOVE_LAST_DIMENSION_IF_NEEDED(X, Y, Z) {{(X), (Y), (Z)}}
#endif

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

        Nullable()
        {
            hasValue = false;
        }
    };
}

#endif /* Core_Headers_Types_h */
