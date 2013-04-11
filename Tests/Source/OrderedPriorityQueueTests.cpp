// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/OrderedPriorityQueueTests.h"
#include "../Headers/Assert.h"

using namespace std;
using namespace Core;

namespace Tests
{
    OrderedPriorityQueue<vector<int>, OrderedPriorityQueueTests::IntComparer> OrderedPriorityQueueTests::eventsQueue;
    vector<int> OrderedPriorityQueueTests::values;

    void OrderedPriorityQueueTests::SetUp()
    {
        int temp[6] = {3, 1, 2, 4, 0, 5};
        values.clear();
        values.insert(values.begin(), temp, temp + 6);

        eventsQueue.Initialize(&values, IntComparer());
    }

    void OrderedPriorityQueueTests::TearDown()
    {

    }

    void OrderedPriorityQueueTests::GetTop_ForOrdered_TopCorrect()
    {
        SetUp();

        int topIndex = eventsQueue.GetTopIndex();

        Assert::AreEqual(topIndex, 4, "GetTop_ForOrdered_TopCorrect");
        TearDown();
    }

    void OrderedPriorityQueueTests::UpdateValue_MakeEachMinimal_TopAlwaysCorrect()
    {
        SetUp();

        int oldTopIndex = eventsQueue.GetTopIndex();
        for (size_t i = 0; i < values.size(); i++)
        {
            int oldValue = values[i];
            values[i] = -1;

            eventsQueue.HandleUpdate(i);
            int topIndex = eventsQueue.GetTopIndex();
            Assert::AreEqual(topIndex, static_cast<int>(i), "UpdateValue_MakeEachMinimal_TopCorrect");

            values[i] = oldValue;
            eventsQueue.HandleUpdate(i);
            topIndex = eventsQueue.GetTopIndex();
            Assert::AreEqual(topIndex, oldTopIndex, "UpdateValue_MakeEachMinimal_TopCorrect");
        }

        TearDown();
    }

    void OrderedPriorityQueueTests::UpdateValue_MakeEachMaximal_TopAlwaysCorrect()
    {
        SetUp();

        for (size_t i = 0; i < values.size() - 1; i++)
        {
            int topIndex = eventsQueue.GetTopIndex();
            values[topIndex] = 100;
            eventsQueue.HandleUpdate(topIndex);

            topIndex = eventsQueue.GetTopIndex();
            Assert::AreEqual(values[topIndex], static_cast<int>(i + 1), "UpdateValue_MakeEachMaximal_TopAlwaysCorrect");
        }

        TearDown();
    }

    void OrderedPriorityQueueTests::RunTests()
    {
        GetTop_ForOrdered_TopCorrect();
        UpdateValue_MakeEachMinimal_TopAlwaysCorrect();
        UpdateValue_MakeEachMaximal_TopAlwaysCorrect();
    }
}


