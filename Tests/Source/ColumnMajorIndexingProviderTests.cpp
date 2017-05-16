// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ColumnMajorIndexingProviderTests.h"

#include "Core/Lattice/Headers/ColumnMajorIndexingProvider.h"
#include "Core/Geometry/Headers/GeometryParameters.h"
#include "../Headers/Assert.h"

using namespace std;
using namespace Core;

namespace Tests
{
    void ColumnMajorIndexingProviderTests::TestGettingSimpleLinearIndex()
    {
        GeometryParameters parameters;
        parameters.SetSystemSize(Axis::X, 10);
        parameters.SetSystemSize(Axis::Y, 10);
        if (DIMENSIONS == 3)
        {
            parameters.SetSystemSize(Axis::Z, 10);
        }

        ColumnMajorIndexingProvider simpleProvider;
        simpleProvider.SetGeometryParameters(&parameters);

        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(7, 6, 8);
        int actualLinearIndex = simpleProvider.GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = (DIMENSIONS == 3) ? 867 : 67;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestGettingSimpleLinearIndex");
    }

    void ColumnMajorIndexingProviderTests::TestGettingComplexLinearIndex()
    {
        GeometryParameters parameters;
        parameters.SetSystemSize(Axis::X, 10);
        parameters.SetSystemSize(Axis::Y, 100);
        if (DIMENSIONS == 3)
        {
            parameters.SetSystemSize(Axis::Z, 1);
        }

        ColumnMajorIndexingProvider complexProvider;
        complexProvider.SetGeometryParameters(&parameters);

        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(9, 99, 0);
        int actualLinearIndex = complexProvider.GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = 10 * 100 - 1;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestGettingComplexLinearIndex");
    }

    void ColumnMajorIndexingProviderTests::TestGettingSimpleMultidimensionalIndex()
    {
        GeometryParameters parameters;
        parameters.SetSystemSize(Axis::X, 10);
        parameters.SetSystemSize(Axis::Y, 10);
        if (DIMENSIONS == 3)
        {
            parameters.SetSystemSize(Axis::Z, 10);
        }

        ColumnMajorIndexingProvider simpleProvider;
        simpleProvider.SetGeometryParameters(&parameters);

        DiscreteSpatialVector multidimensionalIndexes;
        int linearIndex = (DIMENSIONS == 3) ? 867 : 67;
        simpleProvider.FillMultidimensionalIndexes(linearIndex, &multidimensionalIndexes);

        Assert::AreEqual(multidimensionalIndexes[0], 7, "TestGettingSimpleMultidimensionalIndex");
    }

    void ColumnMajorIndexingProviderTests::TestGettingComplexMultidimensionalIndex()
    {
        GeometryParameters parameters;
        parameters.SetSystemSize(Axis::X, 10);
        parameters.SetSystemSize(Axis::Y, 100);
        if (DIMENSIONS == 3)
        {
            parameters.SetSystemSize(Axis::Z, 1);
        }

        ColumnMajorIndexingProvider complexProvider;
        complexProvider.SetGeometryParameters(&parameters);

        DiscreteSpatialVector multidimensionalIndexes;
        int linearIndex = 10 * 100 - 1;
        complexProvider.FillMultidimensionalIndexes(linearIndex, &multidimensionalIndexes);

        Assert::AreEqual(multidimensionalIndexes[0], 9, "TestGettingComplexMultidimensionalIndex");
    }

    void ColumnMajorIndexingProviderTests::TestPeriodicConditions()
    {
        GeometryParameters parameters;
        parameters.SetSystemSize(Axis::X, 3);
        parameters.SetSystemSize(Axis::Y, 10);
        if (DIMENSIONS == 3)
        {
            parameters.SetSystemSize(Axis::Z, 1);
        }

        parameters.SetPeriodicity(Axis::X, true);
        ColumnMajorIndexingProvider provider;
        provider.SetGeometryParameters(&parameters);

        TestPeriodicConditionsBackwards(&provider);
        TestPeriodicConditionsLongBackwards(&provider);
        TestPeriodicConditionsForward(&provider);
        TestPeriodicConditionsLongForward(&provider);
    }

    void ColumnMajorIndexingProviderTests::TestPeriodicConditionsBackwards(ColumnMajorIndexingProvider* provider)
    {
        //-1 should be transformed into 2
        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(-1, 9, 0);
        int actualLinearIndex = provider->GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = 3 * 10 - 1;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestPeriodicConditionsBackwards");
    }

    void ColumnMajorIndexingProviderTests::TestPeriodicConditionsLongBackwards(ColumnMajorIndexingProvider* provider)
    {
        //-3 should be transformed into 0
        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(-3, 0, 0);
        int actualLinearIndex = provider->GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = 0;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestPeriodicConditionsLongBackwards");
    }

    void ColumnMajorIndexingProviderTests::TestPeriodicConditionsForward(ColumnMajorIndexingProvider* provider)
    {
        //3 should be transformed into 0
        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(3, 0, 0);
        int actualLinearIndex = provider->GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = 0;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestPeriodicConditionsLongBackwards");
    }

    void ColumnMajorIndexingProviderTests::TestPeriodicConditionsLongForward(ColumnMajorIndexingProvider* provider)
    {
        //7 should be transformed into 1
        DiscreteSpatialVector multidimensionalIndexes = REMOVE_LAST_DIMENSION_IF_NEEDED(7, 0, 0);
        int actualLinearIndex = provider->GetLinearIndex(multidimensionalIndexes);

        int expectedLinearIndex = 1;

        Assert::AreEqual(actualLinearIndex, expectedLinearIndex, "TestPeriodicConditionsLongForward");
    }

    void ColumnMajorIndexingProviderTests::RunTests()
    {
        TestGettingSimpleLinearIndex();
        TestGettingComplexLinearIndex();
        TestGettingSimpleMultidimensionalIndex();
        TestGettingComplexMultidimensionalIndex();
        TestPeriodicConditions();
    }
}
