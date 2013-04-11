// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_SphericalHarmonicsComputerTests_h
#define Headers_SphericalHarmonicsComputerTests_h

#include "Core/Headers/Types.h"

namespace Tests
{
    class SphericalHarmonicsComputerTests
    {
    public:
        static void RunTests();
    private:
        static void SetUp();
        static void TearDown();

        static void GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsZero_IsCorrect();
        static void GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsOne_IsCorrect();
        static void GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsMinusOne_IsCorrect();
        static void GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsZero_IsCorrect();

        static Core::FLOAT_TYPE GetSphericalHarmonicsError(int l, int m);
    };
}

#endif /* Headers_SphericalHarmonicsComputerTests_h */
