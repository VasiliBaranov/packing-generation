// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Math.h"
#include "../Headers/VectorUtilities.h"
#include "../Headers/StlUtilities.h"
#include "../Headers/Constants.h"

using namespace std;

namespace Core
{
    bool Math::spareReady = false;
    FLOAT_TYPE Math::spare = 0;

    bool Math::IsNumber(FLOAT_TYPE x)
    {
        // This looks like it should always be true, 
        // but it's false if x is a NaN.
        return (x == x); 
    }

    bool Math::IsFiniteNumber(FLOAT_TYPE x)
    {
        return (x <= MAX_FLOAT_VALUE && x >= -MAX_FLOAT_VALUE);
    }

    int Math::Round(FLOAT_TYPE x)
    {
        if(x >= 0)
        {
            return static_cast<int>(x + 0.5); //floor
        }
        else
        {
            return static_cast<int>(x - 0.5);
        }
    }

    void Math::SetSeed(int seed)
    {
#ifdef WINDOWS
        srand(seed);
#else
        srand48(seed);
#endif
    }

    FLOAT_TYPE Math::GetNextRandom()
    {
#ifdef WINDOWS
        return rand() / static_cast<FLOAT_TYPE>(RAND_MAX);
#else
        return drand48();
#endif
    }

    // See http://en.wikipedia.org/wiki/Marsaglia_polar_method
    FLOAT_TYPE Math::GetNextGaussianRandom(double mean, double standardDeviation)
    {
        if (spareReady)
        {
            spareReady = false;
            return mean + standardDeviation * spare;
        }
        else
        {
            double x, y, radiusSquare;
            do
            {
                x = GetNextRandom() * 2.0 - 1.0;
                y = GetNextRandom() * 2.0 - 1.0;
                radiusSquare = x * x + y * y;
            } while (radiusSquare >= 1.0);

            FLOAT_TYPE multiplier;
            if (radiusSquare == 0.0)
            {
                multiplier = 0.0;
            }
            else
            {
                multiplier = sqrt(-2.0 * log(radiusSquare) / radiusSquare);
            }

            spare = y * multiplier;
            spareReady = true;
            return mean + standardDeviation * x * multiplier;
        }
    }

    void Math::CalculateStepSize(FLOAT_TYPE minBinLeftEdge, FLOAT_TYPE maxBinRightEdge, FLOAT_TYPE expectedStep, FLOAT_TYPE* actualStep, int* binsCount)
    {
        int& binsCountRef = *binsCount;
        FLOAT_TYPE& actualStepRef = *actualStep;

        binsCountRef = Math::Round((maxBinRightEdge - minBinLeftEdge) / expectedStep);
        actualStepRef = (maxBinRightEdge - minBinLeftEdge) / binsCountRef;
    }

    void Math::DistributeValuesToBins(const vector<FLOAT_TYPE>& values, FLOAT_TYPE step,
                    vector<FLOAT_TYPE>* binLeftEdges, vector<vector<int> >* mappingToBins)
    {
        binLeftEdges->clear();
        mappingToBins->clear();

        FLOAT_TYPE minValue = VectorUtilities::GetMinValue(values);
        FLOAT_TYPE maxValue = VectorUtilities::GetMaxValue(values);

        int binsCount;
        CalculateStepSize(minValue, maxValue, step, &step, &binsCount);
        binLeftEdges->resize(binsCount);
        mappingToBins->resize(binsCount);

        VectorUtilities::FillLinearScale(minValue, maxValue - step, binLeftEdges);

        int valueIndex = 0;
        for (std::vector<FLOAT_TYPE>::const_iterator it = values.begin(); it != values.end(); ++it)
        {
            FLOAT_TYPE value = *it;
            int binIndex = static_cast<int>(std::floor((value - minValue) / step));

            // maxValue will have binIndex == binsCount
            if (binIndex == binsCount)
            {
                binIndex--;
            }
            mappingToBins->at(binIndex).push_back(valueIndex);
            valueIndex++;
        }
    }

    void Math::EnsureMinCountInBins(int minValuesCountInBin, vector<FLOAT_TYPE>* binLeftEdges, vector<vector<int> >* mappingToBins)
    {
        int binsCount = mappingToBins->size();
        int binIndex = 0;
        while (binIndex < binsCount - 1) // binsCount will for sure be > 1
        {
            vector<int>& mappingToBin = mappingToBins->at(binIndex);
            while (mappingToBin.size() < static_cast<size_t>(minValuesCountInBin) && binIndex < binsCount - 1)
            {
                vector<int>& mappingToNextBin = mappingToBins->at(binIndex + 1);
                StlUtilities::Append(mappingToNextBin, &mappingToBin);

                mappingToBins->erase(mappingToBins->begin() + binIndex + 1);
                binLeftEdges->erase(binLeftEdges->begin() + binIndex + 1);
                binsCount--;
            }
            binIndex++;
        }

        // Process the last bin. It may still contain less than the required amount of items.
        if (binsCount > 1)
        {
            binIndex = binsCount - 1;
            vector<int>& mappingToLastBin = mappingToBins->at(binsCount - 1);
            if (mappingToLastBin.size() < static_cast<size_t>(minValuesCountInBin))
            {
                vector<int>& mappingToPreviousBin = mappingToBins->at(binIndex - 1);
                StlUtilities::Append(mappingToLastBin, &mappingToPreviousBin);
                mappingToBins->erase(mappingToBins->begin() + binIndex);
                binLeftEdges->erase(binLeftEdges->begin() + binIndex);
            }
        }
    }

    void Math::ConvertBinLeftEdgesToCenters(vector<FLOAT_TYPE>* binLeftEdges, FLOAT_TYPE maxRightEdge)
    {
        vector<FLOAT_TYPE>& binLeftEdgesRef = *binLeftEdges;
        int size = binLeftEdges->size();
        if (size > 1)
        {
            for (int binIndex = 0; binIndex < size - 1; ++binIndex)
            {
                binLeftEdgesRef[binIndex] = (binLeftEdgesRef[binIndex] + binLeftEdgesRef[binIndex + 1]) * 0.5;
            }
        }

        binLeftEdgesRef[size - 1] = (binLeftEdgesRef[size - 1] + maxRightEdge) * 0.5;
    }
}

