// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_Assert_h
#define Headers_Assert_h

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

namespace Tests
{
    // Includes methods for asserting various statements.
    class Assert
    {
    public:
        template<class T>
        static void AreAlmostEqual(T actualValue, T expectedValue, std::string callerName, double epsilon = 1e-5)
        {
            if ((actualValue != actualValue) ||
                    (expectedValue != expectedValue) ||
                    std::abs(actualValue - expectedValue) > epsilon * (std::abs(actualValue) + std::abs(expectedValue)) * 0.5) // both can not be zero
            {
                std::cout << "Assertion failed for " << callerName << ".\n";
                exit(EXIT_FAILURE);
            }
        }

        template<class TContainerX, class TContainerY>
        static void AreVectorsAlmostEqual(const TContainerX& actualValues, const TContainerY& expectedValues, std::string callerName)
        {
            for (size_t i = 0; i < actualValues.size(); i++)
            {
                AreAlmostEqual(actualValues[i], expectedValues[i], callerName);
            }
        }

        template<class T>
        static void AreMatricesAlmostEqual(T** actualValues, T** expectedValues, int dimensions, std::string callerName)
        {
            for (int i = 0; i < dimensions; i++)
            {
                for (int j = 0; j < dimensions; j++)
                {
                    AreAlmostEqual(actualValues[i][j], expectedValues[i][j], callerName);
                }
            }
        }

        template<class T>
        static void AreEqual(T actualValue, T expectedValue, std::string callerName)
        {
            if (actualValue != expectedValue)
            {
                std::cout << "Assertion failed for " << callerName << ".\n";
                exit( EXIT_FAILURE );
            }
        }

        template<class T>
        static void AreNotEqual(T actualValue, T expectedValue, std::string callerName)
        {
            if (actualValue == expectedValue)
            {
                std::cout << "Assertion failed for " << callerName << ".\n";
                exit( EXIT_FAILURE );
            }
        }

        template<class TContainerX, class TContainerY>
        static void AreVectorsEqual(const TContainerX& actualValues, const TContainerY& expectedValues, std::string callerName)
        {
            for (size_t i = 0; i < actualValues.size(); i++)
            {
                AreEqual(actualValues[i], expectedValues[i], callerName);
            }
        }

        template<class T> static void IsLessThanZero(T actualValue, std::string callerName)
        {
            if (actualValue >= 0)
            {
                std::cout << "Assertion failed for " << callerName << ".\n";
                exit( EXIT_FAILURE );
            }
        }

        template<class T> static void IsGreaterThanZero(T actualValue, std::string callerName)
        {
            if (actualValue <= 0)
            {
                std::cout << "Assertion failed for " << callerName << ".\n";
                exit( EXIT_FAILURE );
            }
        }

        static void IsTrue(bool value, std::string callerName);

        static void WaitForKey();

    };
}

#endif /* Headers_Assert_h */
