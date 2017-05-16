// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Math_h
#define Core_Headers_Math_h

#include <vector>
#include "Types.h"

#ifdef WINDOWS
#define NOMINMAX
#include <windows.h>
#undef DeleteFile
#undef GetMessage
#else
#include <unistd.h>
#endif

namespace Core
{
    // Implements math utility methods.
    class Math
    {
    private:
        static FLOAT_TYPE spare;
        static bool spareReady;

    public:
        //Determines, whether x is a number (i.e. not a NaN or infinity)
        static bool IsNumber(FLOAT_TYPE x);

        //Determines, whether x is a finite number.
        static bool IsFiniteNumber(FLOAT_TYPE x);

        //Rounds the input.
        static int Round(FLOAT_TYPE x);

        static void SetSeed(int seed);

        static FLOAT_TYPE GetNextRandom();

        static void CalculateStepSize(FLOAT_TYPE minBinLeftEdge, FLOAT_TYPE maxBinRightEdge, FLOAT_TYPE expectedStep, FLOAT_TYPE* actualStep, int* binsCount);

        static void DistributeValuesToBins(const std::vector<FLOAT_TYPE>& values, FLOAT_TYPE step,
                std::vector<FLOAT_TYPE>* binLeftEdges, std::vector<std::vector<int> >* mappingToBins);

        static void EnsureMinCountInBins(int minValuesCountInBin, std::vector<FLOAT_TYPE>* binLeftEdges, std::vector<std::vector<int> >* mappingToBins);

        template<class TItem>
        static void DistributeItemsToBins(const std::vector<TItem>& items, const std::vector<std::vector<int> >* mappingToBins, std::vector<std::vector<TItem> >* mappedItems)
        {
            mappedItems->clear();
            mappedItems->resize(mappingToBins->size());

            for (int binIndex = 0; binIndex < mappingToBins->size(); ++binIndex)
            {
                const std::vector<int>& indexesInBin = mappingToBins->at(binIndex);
                std::vector<TItem>& itemsInBin = mappedItems->at(binIndex);

                for (std::vector<int>::const_iterator it = indexesInBin.begin(); it != indexesInBin.end(); ++it)
                {
                    int valueIndex = *it;
                    const TItem& item = items->at(valueIndex);
                    itemsInBin.push_back(item);
                }
            }
        }

        static void ConvertBinLeftEdgesToCenters(std::vector<FLOAT_TYPE>* binLeftEdges, FLOAT_TYPE maxRightEdge);

        static FLOAT_TYPE GetNextGaussianRandom(double mean, double stdDev);
    };
}

#endif /* Core_Headers_Math_h */
