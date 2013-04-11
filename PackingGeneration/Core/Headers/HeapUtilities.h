// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_HeapUtilities_h
#define Core_Headers_HeapUtilities_h

#include <vector>
#include "VectorUtilities.h"

namespace Core
{
    // Provides methods to make binary heaps over arrays or stl containers. See http://en.wikipedia.org/wiki/Binary_heap.
    // Stl has methods make_heap, push_heap, pop_heap, but they do not track elements permutations, and stl doesn't have a method HandleUpdate.
    // This class is implemented as a static utility class to conform to stl style and to be applicable to any stl container, as well as pure arrays.
    // This design doesn't allow replacing heaps with stubs or mocks in tests, but such basic data structures are rarely replaced with stubs in tests.
    class HeapUtilities
    {
    public:
        // Rearranges the elements in the range [first,lastExclusive) in such a way that they form a heap.
        // In order to rearrange these elements, the function performs comparisons using compare functor.
        // TGetInitialIndex is the functor that accepts an element of the container and returns its initial index in the container.
        // You may implement it through passing a vector of tuples, or through maintaining a separate stl::map.
        // There is a trivial case, when the container is a vector of ints, where each int is the initial position itself.
        // Returns a permutation array p (i.e. if p[13] == 0, then the element at index 13 in the initial array is now at index 0).
        template <class TRandomAccessIterator, class TCompare, class TGetInitialIndex>
        static void Make(TRandomAccessIterator first, TRandomAccessIterator lastExclusive, TCompare lessThan, TGetInitialIndex getInitialIndex, std::vector<int>* permutation)
        {
            int count = GetHeapLength(first, lastExclusive);
            permutation->resize(count);
            VectorUtilities::FillLinearScale(0, permutation);

            int lastNonLeafIndex = count / 2 - 1;
            for (int nodeIndex = lastNonLeafIndex; nodeIndex >= 0; nodeIndex--)
            {
                EnsureHeapProperty(first, lastExclusive, lessThan, getInitialIndex, nodeIndex, permutation);
            }
        }

        template <class TRandomAccessIterator, class TCompare, class TGetInitialIndex>
        static void HandleUpdate(TRandomAccessIterator first, TRandomAccessIterator lastExclusive, TCompare lessThan, TGetInitialIndex getInitialIndex, int nodeIndex, std::vector<int>* permutation)
        {
            bool swapsOccurred = SwapWithParents(first, lessThan, getInitialIndex, nodeIndex, permutation);

            if (!swapsOccurred)
            {
                EnsureHeapProperty(first, lastExclusive, lessThan, getInitialIndex, nodeIndex, permutation);
            }
        }

    private:
        template <class TRandomAccessIterator, class TCompare, class TGetInitialIndex>
        static bool SwapWithParents(TRandomAccessIterator first, TCompare lessThan, TGetInitialIndex getInitialIndex, int nodeIndex, std::vector<int>* permutation)
        {
            bool swapsOccurred = false;
            int parentIndex = GetParentIndex(nodeIndex);
            while (parentIndex >= 0)
            {
                bool shouldSwap = lessThan(first[nodeIndex], first[parentIndex]);
                if (!shouldSwap)
                {
                    break;
                }

                swapsOccurred = true;
                SwapValuesAndIndexes(first, getInitialIndex, parentIndex, nodeIndex, permutation);
                nodeIndex = parentIndex;
                parentIndex = GetParentIndex(parentIndex);
            }

            return swapsOccurred;
        }

        template <class TRandomAccessIterator, class TCompare, class TGetInitialIndex>
        static void EnsureHeapProperty(TRandomAccessIterator first, TRandomAccessIterator lastExclusive, TCompare lessThan, TGetInitialIndex getInitialIndex, int nodeIndex, std::vector<int>* permutation)
        {
            int count = GetHeapLength(first, lastExclusive);
            int leftChildIndex = GetLeftChildIndex(nodeIndex);
            int rightChildIndex = GetRightChildIndex(nodeIndex);

            int smallestIndex = nodeIndex;

            if (leftChildIndex < count && lessThan(first[leftChildIndex], first[smallestIndex]))
            {
                smallestIndex = leftChildIndex;
            }

            if (rightChildIndex < count && lessThan(first[rightChildIndex], first[smallestIndex]))
            {
                smallestIndex = rightChildIndex;
            }

            if (smallestIndex != nodeIndex)
            {
                SwapValuesAndIndexes(first, getInitialIndex, nodeIndex, smallestIndex, permutation);
                EnsureHeapProperty(first, lastExclusive, lessThan, getInitialIndex, smallestIndex, permutation);
            }
        }

        template <class TRandomAccessIterator, class TGetInitialIndex>
        static void SwapValuesAndIndexes(TRandomAccessIterator first, TGetInitialIndex getInitialIndex, int firstIndex, int secondIndex, std::vector<int>* permutation)
        {
            std::vector<int>& permutationRef = *permutation;
            int currentIndexAtFirstPosition = getInitialIndex(first[firstIndex]);
            int currentIndexAtSecondPosition = getInitialIndex(first[secondIndex]);
            permutationRef[currentIndexAtFirstPosition] = secondIndex;
            permutationRef[currentIndexAtSecondPosition] = firstIndex;

            SwapValues(first, firstIndex, secondIndex);
        }

        template <class TRandomAccessIterator>
        static void SwapValues(TRandomAccessIterator first, int firstIndex, int secondIndex)
        {
            typedef typename std::iterator_traits<TRandomAccessIterator>::value_type TValueType;
            TValueType temp = first[firstIndex];
            first[firstIndex] = first[secondIndex];
            first[secondIndex] = temp;
        }

        template <class TRandomAccessIterator>
        static int GetHeapLength(TRandomAccessIterator first, TRandomAccessIterator lastExclusive)
        {
            return lastExclusive - first;
        }

        static int GetLeftChildIndex(int nodeIndex)
        {
            return 2 * nodeIndex + 1;
        }

        static int GetRightChildIndex(int nodeIndex)
        {
            return 2 * nodeIndex + 2;
        }

        static int GetParentIndex(int index)
        {
            // Floor is applied in C++
            return (index - 1) / 2;
        }
    };
}

#endif /* Core_Headers_HeapUtilities_h */
