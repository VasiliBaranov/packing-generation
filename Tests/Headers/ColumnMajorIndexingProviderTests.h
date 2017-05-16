// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_ColumnMajorIndexingProviderTests_h
#define Headers_ColumnMajorIndexingProviderTests_h

namespace Core { class ColumnMajorIndexingProvider; }

namespace Tests
{
    //Defines methods for testing column major indexing provider test.
    //TODO: rename methods
    class ColumnMajorIndexingProviderTests
    {
    private:
        static void TestGettingSimpleLinearIndex();
        static void TestGettingComplexLinearIndex();
        static void TestGettingSimpleMultidimensionalIndex();
        static void TestGettingComplexMultidimensionalIndex();
        static void TestPeriodicConditions();

        static void TestPeriodicConditionsBackwards(Core::ColumnMajorIndexingProvider* provider);
        static void TestPeriodicConditionsLongBackwards(Core::ColumnMajorIndexingProvider* provider);
        static void TestPeriodicConditionsForward(Core::ColumnMajorIndexingProvider* provider);
        static void TestPeriodicConditionsLongForward(Core::ColumnMajorIndexingProvider* provider);

    public:
        static void RunTests();
    };
}

#endif /* Headers_ColumnMajorIndexingProviderTests_h */
