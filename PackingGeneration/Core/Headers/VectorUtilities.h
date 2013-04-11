// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_VectorUtilities_h
#define Core_Headers_VectorUtilities_h

#include <cmath>
#include <algorithm>

// Provides static utility methods for dealing with vectors.

// Why didn't introduce a custom array class:
// 1. it will suffer from the following problem: some of these methods logically belong both to matrices and to vectors
// (that's why Services are introduced along with Model classes into Business Logic Layer in Domain Driven Design).
// 2. calls like v1 = v1 + v1 * v2; will require numerous useless copy constructor calls and memory overhead.
// C++11 move constructors solve this issue, but C++11 is not available on my supercomputers currently.

// I also tried introducing BaseVectorUtilities, whose methods accept iterators in the STL style,
// e.g., static void Add(TIteratorTo toBegin, TIteratorTo toEnd, TIteratorValue value, TIteratorResult result),
// and passing calls from VectorUtilities to BaseVectorUtilities (e.g. BaseVectorUtilities::Add(x.begin(), x.end(), y.begin(), result->begin());

// Advantages of BaseVectorUtilities:
// 1. allows passing pure C arrays and stl lists to BaseVectorUtilities

// Advantages of NO BaseVectorUtilities
// 1. simpler code
// 2. better performance
// 3. prevents usage of C arrays, of stl lists

// Below are performance measurements (for different BaseVectorUtilities implementations) of PackingGeneration.exe runs on a local system,
// number of particles = 10000, initial packing density = 0.6 (FBA packing), compression rate is 1, requires 27 iterations, Lubachevsky-Stillinger generation, g++ 4.5.3, -O3:
// 1. current version (no BaseVectorUtilities) - 29.1 seconds
// 2. BaseVectorUtilities with while loops (e.g. while (toBegin != toEnd) { (*result) = (*toBegin) + (*value); ++toBegin; ++value; ++result; }) - 32.8 seconds (+12%)
// 3. BaseVectorUtilities with stl::transform calls (e.g. std::transform(toBegin, toEnd, value, result, std::plus<TItemType>());) - 32.9 seconds
// 4. BaseVectorUtilities with for loops (e.g. const std::size_t size = toEnd - toBegin; for (int i = 0; i < size; ++i) { result[i] = toBegin[i] + value[i]; }) - 34.117 seconds
namespace Core
{
#define CONTAINER_VALUE_TYPE(TContainer)typename TContainer::value_type
    class VectorUtilities
    {
    public:
        //////////////////////////////////
        // Zipping functions: vector x vector -> vector or vector x value -> vector
        /////////////////////////////////

        // Multiplies vectors element-per-element, stores the result in the result array.
        template<class TContainerX, class TContainerY, class TContainerResult>
        static void Multiply(const TContainerX& x, const TContainerY& y, TContainerResult* result)
        {
            const std::size_t size = x.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = x[i] * y[i];
            }
        }

        // Multiplies the vector by a multiplier, stores the result in the result array.
        template<class TContainer, class TContainerResult>
        static void MultiplyByValue(const TContainer& vector, const CONTAINER_VALUE_TYPE(TContainer)& multiplier, TContainerResult* result)
        {
            const std::size_t size = vector.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = vector[i] * multiplier;
            }
        }

        // Divides vectors element-per-element, stores the result in the result array.
        template<class TContainerDividend, class TContainerDivisor, class TContainerResult>
        static void Divide(const TContainerDividend& x, const TContainerDivisor& y, TContainerResult* result)
        {
            const std::size_t size = x.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = x[i] / y[i];
            }
        }

        // Divides the vector by a divisor, stores the result in the result array.
        template<class TContainerDividend, class TContainerResult>
        static void DivideByValue(const TContainerDividend& dividend, const CONTAINER_VALUE_TYPE(TContainerDividend)& divisor, TContainerResult* result)
        {
            const std::size_t size = dividend.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = dividend[i] / divisor;
            }
        }

        // Sums two vectors, stores the result in the result array.
        template<class TContainerTo, class TContainerValue, class TContainerResult>
        static void Add(const TContainerTo& x, const TContainerValue& y, TContainerResult* result)
        {
            const std::size_t size = x.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = x[i] + y[i];
            }
        }

        template<class TContainerTo, class TContainerResult>
        static void AddValue(TContainerTo x, const CONTAINER_VALUE_TYPE(TContainerTo)& y, TContainerResult* result)
        {
            const std::size_t size = x.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = x[i] + y;
            }
        }

        // Subtracts two vectors, stores the result (from - value) in the result array.
        template<class TContainerFrom, class TContainerValue, class TContainerResult>
        static void Subtract(const TContainerFrom& from, const TContainerValue& value, TContainerResult* result)
        {
            const std::size_t size = from.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = from[i] - value[i];
            }
        }

        template<class TContainerFrom, class TContainerResult>
        static void SubtractValue(const TContainerFrom& from, const CONTAINER_VALUE_TYPE(TContainerFrom)& value, TContainerResult* result)
        {
            const std::size_t size = from.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = from[i] - value;
            }
        }

        template<class TContainerX, class TContainerY, class TContainerResult>
        static void Mod(const TContainerX& vector, const TContainerY& base, TContainerResult* result)
        {
            const std::size_t size = vector.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = vector[i] % base[i];
            }
        }

        //////////////////////////////////
        // Mapping functions: vector -> vector
        /////////////////////////////////

        template<class TContainer, class TContainerResult>
        static void Abs(const TContainer& vector, TContainerResult* result)
        {
            const std::size_t size = vector.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = std::abs(vector[i]);
            }
        }

        template<class TContainer, class TContainerResult>
        static void Floor(const TContainer& vector, TContainerResult* result)
        {
            const std::size_t size = vector.size();
            TContainerResult& resultRef = *result;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = std::floor(vector[i]);
            }
        }

        template<class TContainer, class TContainerResult>
        static void Convert(const TContainer& vector, TContainerResult* result)
        {
            const std::size_t size = vector.size();
            TContainerResult& resultRef = *result;
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            for (std::size_t i = 0; i < size; ++i)
            {
                resultRef[i] = static_cast<TItemType>(vector[i]);
            }
        }

        //////////////////////////////////
        // Aggregate/reduce functions: vector -> value
        /////////////////////////////////

        // Gets the dot product of vectors
        template<class TContainerX, class TContainerY>
        static CONTAINER_VALUE_TYPE(TContainerX) GetDotProduct(const TContainerX& x, const TContainerY& y)
        {
            typedef CONTAINER_VALUE_TYPE(TContainerX) TItemType;
            TItemType dotProduct = TItemType(0);
            const std::size_t size = x.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                dotProduct += x[i] * y[i];
            }
            return dotProduct;
        }

        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) GetSelfDotProduct(const TContainer& x)
        {
            return GetDotProduct(x, x);
        }

        // Gets the length of the vector.
        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) GetLength(const TContainer& vector)
        {
            return sqrt(GetSelfDotProduct(vector));
        }

        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) Sum(const TContainer& vector)
        {
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            TItemType sum = TItemType(0);
            const std::size_t size = vector.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                sum += vector[i];
            }
            return sum;
        }

        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) GetProduct(const TContainer& vector)
        {
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            TItemType sum = TItemType(1);
            const std::size_t size = vector.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                sum *= vector[i];
            }
            return sum;
        }

        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) GetMaxValue(const TContainer& vector)
        {
            typename TContainer::const_iterator it = std::max_element(vector.begin(), vector.end());
            return *it;
        }

        template<class TContainer>
        static CONTAINER_VALUE_TYPE(TContainer) GetMinValue(const TContainer& vector)
        {
            typename TContainer::const_iterator it = std::min_element(vector.begin(), vector.end());
            return *it;
        }

        //////////////////////////////////
        // Initialize functions: vector -> vector (actually it is semantically [] -> vector)
        /////////////////////////////////

        template<class TContainer>
        static void InitializeWith(TContainer* vector, const CONTAINER_VALUE_TYPE(TContainer)& value)
        {
            std::fill(vector->begin(), vector->end(), value);
        }

        template<class TContainer>
        static void InitializeWithRandoms(TContainer* vector)
        {
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            TContainer& vectortRef = *vector;
            const std::size_t size = vectortRef.size();
            for (std::size_t i = 0; i < size; ++i)
            {
#ifdef WINDOWS
                vectortRef[i] = TItemType(rand()) / RAND_MAX;
#else
                vectortRef[i] = TItemType(drand48());
#endif
            }
        }

        template<class TContainer>
        static void FillLogScale(const CONTAINER_VALUE_TYPE(TContainer)& minValue, const CONTAINER_VALUE_TYPE(TContainer)& maxValue, TContainer* values)
        {
            // x_i = c * exp(b * i), then ln x_i = a + b * i
            // c = x_0, b = ln(x_N / x_0) / N, x_i = x_0 * (x_N / x_0) ^ (i / N)

            TContainer& valuestRef = *values;
            const std::size_t size = valuestRef.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                valuestRef[i] = minValue * pow(maxValue / minValue, i * 1.0 / (size - 1));
            }
        }

        template<class TContainer>
        static void FillLinearScale(const CONTAINER_VALUE_TYPE(TContainer)& minBinLeftEdge, const CONTAINER_VALUE_TYPE(TContainer)& maxBinLeftEdge, TContainer* values)
        {
            // step = (maxBinRightEdge - minBinLeftEdge) / binsCount;
            // step = (maxBinLeftEdge + step - minBinLeftEdge) / binsCount;
            // step * binsCount - step = maxBinLeftEdge - minBinLeftEdge;
            // step = (maxBinLeftEdge - minBinLeftEdge) / (binsCount - 1);

            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            TContainer& valuestRef = *values;
            const std::size_t binsCount = valuestRef.size();
            TItemType step = (maxBinLeftEdge - minBinLeftEdge) / (binsCount - 1);
            TItemType currentValue = minBinLeftEdge;
            for (std::size_t i = 0; i < binsCount; ++i)
            {
                valuestRef[i] = currentValue;
                // May try to remove extra-copy constructor call :-)
                currentValue += step;
            }
        }

        template<class TContainer>
        static void FillLinearScale(const CONTAINER_VALUE_TYPE(TContainer)& minValue, TContainer* values)
        {
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            TItemType currentValue = minValue;
            TContainer& valuestRef = *values;
            const std::size_t size = valuestRef.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                valuestRef[i] = currentValue;
                currentValue += TItemType(1);
            }
        }

        //////////////////////////////
        // Matrix utilities
        //////////////////////////////

        template<class TMatrix, class T>
        static T GetTrace(TMatrix matrix, int numberOfDimensions)
        {
            T trace = 0.0;
            for (int i = 0; i < numberOfDimensions; ++i)
            {
                trace += matrix[i][i];
            }
            return trace;
        }

        template<class TMatrix, class T>
        static void InitializeWith(TMatrix matrix, T value, int numberOfRows, int numberOfColumns)
        {
            for (int rowIndex = 0; rowIndex < numberOfRows; ++rowIndex)
            {
                for (int columnIndex = 0; columnIndex < numberOfColumns; ++columnIndex)
                {
                    matrix[rowIndex][columnIndex] = value;
                }
            }
        }

        template<class TMatrix>
        static void Transpose(TMatrix matrix, TMatrix transposedMatrix, int dimensions)
        {
            for (int rowIndex = 0; rowIndex < dimensions; ++rowIndex)
            {
                for (int columnIndex = 0; columnIndex < dimensions; ++columnIndex)
                {
                    transposedMatrix[rowIndex][columnIndex] = matrix[columnIndex][rowIndex];
                }
            }
        }

        template<class TMatrix>
        static void Add(TMatrix x, TMatrix y, TMatrix result, int numberOfRows, int numberOfColumns)
        {
            for (int rowIndex = 0; rowIndex < numberOfRows; ++rowIndex)
            {
                for (int columnIndex = 0; columnIndex < numberOfColumns; ++columnIndex)
                {
                    result[rowIndex][columnIndex] = x[rowIndex][columnIndex] + y[rowIndex][columnIndex];
                }
            }
        }

        template<class TMatrix, class TVector>
        static void Multiply(TMatrix matrix, TVector vector, TVector result, int numberOfRows, int numberOfColumns)
        {
            for (int rowIndex = 0; rowIndex < numberOfRows; ++rowIndex)
            {
                result[rowIndex] = 0;
                for (int columnIndex = 0; columnIndex < numberOfColumns; ++columnIndex)
                {
                   result[rowIndex] += matrix[rowIndex][columnIndex] * vector[columnIndex];
                }
            }
        }

        template<class TMatrix, class T>
        static void Multiply(TMatrix matrix, T multiplier, TMatrix result, int numberOfRows, int numberOfColumns)
         {
             for (int rowIndex = 0; rowIndex < numberOfRows; ++rowIndex)
             {
                 for (int columnIndex = 0; columnIndex < numberOfColumns; ++columnIndex)
                 {
                    result[rowIndex][columnIndex] = matrix[rowIndex][columnIndex] * multiplier;
                 }
             }
         }

        template<class TMatrix, class TVector, class T>
        static T ComputeBilinearForm(TMatrix formMatrix, TVector x, TVector y, int dimensions)
        {
            T result = 0;
            for (int rowIndex = 0; rowIndex < dimensions; ++rowIndex)
            {
                for (int columnIndex = 0; columnIndex < dimensions; ++columnIndex)
                {
                    result += x[rowIndex] * formMatrix[rowIndex][columnIndex] * y[columnIndex];
                }
            }

            return result;
        }

        static int GetColumnMajorIndex(int rowsCount, int columnsCount, int row, int column);

        static int GetRowMajorIndex(int rowsCount, int columnsCount, int row, int column);

        static int GetKroneckerDelta(int i, int j);
    };
}

#endif /* Core_Headers_VectorUtilities_h */
