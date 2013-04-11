// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/SphericalHarmonicsComputerTests.h"

#include "../Headers/Assert.h"
#include "Core/Headers/SphericalHarmonicsComputer.h"

using namespace std;
using namespace Core;

namespace Tests
{
    void SphericalHarmonicsComputerTests::SetUp()
    {

    }

    void SphericalHarmonicsComputerTests::TearDown()
    {

    }

    void SphericalHarmonicsComputerTests::GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsZero_IsCorrect()
    {
        SetUp();

        FLOAT_TYPE error = GetSphericalHarmonicsError(0, 0);
        Assert::AreAlmostEqual(error, 0.0, "GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsZero_IsCorrect");

        TearDown();
    }

    void SphericalHarmonicsComputerTests::GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsOne_IsCorrect()
    {
        SetUp();

        FLOAT_TYPE error = GetSphericalHarmonicsError(1, 1);
        Assert::AreAlmostEqual(error, 0.0, "GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsOne_IsCorrect");

        TearDown();
    }

    void SphericalHarmonicsComputerTests::GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsMinusOne_IsCorrect()
    {
        SetUp();

        FLOAT_TYPE error = GetSphericalHarmonicsError(1, -1);
        Assert::AreAlmostEqual(error, 0.0, "GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsOne_IsCorrect");

        TearDown();
    }

    void SphericalHarmonicsComputerTests::GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsZero_IsCorrect()
    {
        SetUp();

        FLOAT_TYPE error = GetSphericalHarmonicsError(1, 0);
        Assert::AreAlmostEqual(error, 0.0, "GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsZero_IsCorrect");

        TearDown();
    }

//    thetas = 0 : 0.1 * pi : pi;
//    phis = 0 : 0.1 * pi : 2 * pi;
//    actualValues = zeros(length(thetas), length(phis));
//    expectedValues = zeros(length(thetas), length(phis));
//
//    l = 1; m = -1;
//    for i = 1 : length(thetas)
//        for j = 1 : length(phis)
//            theta = thetas(i);
//            phi = phis(j);
//            currentValues = SphericalHarmonicsComputer.GetSphericalHarmonicValues(theta, phi, l);
//            actualValues(i, j) = currentValues(m + 1 + l);
//            expectedValues(i, j) = SphericalHarmonicsComputer.GetAnalyticSphericalHarmonicValue(theta, phi, l, m);
//        end
//    end
//    difference = actualValues - expectedValues;
//    norm(difference)
    FLOAT_TYPE SphericalHarmonicsComputerTests::GetSphericalHarmonicsError(int l, int m)
    {
        const int thetasCount = 100;
        const int phisCount = 100;
        vector<FLOAT_TYPE> thetas(thetasCount);
        vector<FLOAT_TYPE> phis(phisCount);

        for (int i = 0; i < thetasCount; i++)
        {
            thetas[i] = PI * i / thetasCount;
        }

        for (int i = 0; i < phisCount; i++)
        {
            phis[i] = PI * i / phisCount;
        }

        FLOAT_TYPE error = 0.0;
        int sphericalHarmonicsCount = 2 * l + 1;
        vector< complex<FLOAT_TYPE> > harmonicValues(sphericalHarmonicsCount);

        for (int i = 0; i < thetasCount; i++)
        {
            for (int j = 0; j < phisCount; j++)
            {
                FLOAT_TYPE theta = thetas[i];
                FLOAT_TYPE phi = phis[j];

                SphericalHarmonicsComputer::FillSphericalHarmonicValues(theta, phi, l, &harmonicValues);
                complex<FLOAT_TYPE> actualValue = harmonicValues[m + l];
                complex<FLOAT_TYPE> expectedValue = SphericalHarmonicsComputer::GetAnalyticalSphericalHarmonicValue(theta, phi, l, m);
                error += norm(actualValue - expectedValue);
            }
        }

        return error;
    }

    void SphericalHarmonicsComputerTests::RunTests()
    {
        GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsZero_IsCorrect();
        GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsOne_IsCorrect();
        GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsMinusOne_IsCorrect();
        GetSphericalHarmonicValues_CompareWithAnalyticalForm_lIsOne_mIsZero_IsCorrect();
    }
}


