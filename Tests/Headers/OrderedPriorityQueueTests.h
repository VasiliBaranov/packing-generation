// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_OrderedPriorityQueueTests_h
#define Headers_OrderedPriorityQueueTests_h

#include "Core/Headers/OrderedPriorityQueue.h"

namespace Tests
{
    class OrderedPriorityQueueTests
    {
    private:
        class IntComparer
        {
        public:
            bool operator()(const int& first, const int& second)
            {
                return first < second;
            };
        };

    private:
        static Core::OrderedPriorityQueue<std::vector<int>, IntComparer> eventsQueue;
        static std::vector<int> values;

    public:
        static void RunTests();

    private:
        static void SetUp();

        static void TearDown();

        static void GetTop_ForOrdered_TopCorrect();

        static void UpdateValue_MakeEachMinimal_TopAlwaysCorrect();

        static void UpdateValue_MakeEachMaximal_TopAlwaysCorrect();
    };
}

#endif /* Headers_OrderedPriorityQueueTests_h */
