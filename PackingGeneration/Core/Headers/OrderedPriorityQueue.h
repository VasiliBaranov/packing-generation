// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_OrderedPriorityQueue_h
#define Core_Headers_OrderedPriorityQueue_h

#include "BaseOrderedPriorityQueue.h"

namespace Core
{
    template<class TContainer, class TCompare>
    class OrderedPriorityQueue
    {
    private:
        BaseOrderedPriorityQueue<typename TContainer::iterator, TCompare> queue;

    public:
        OrderedPriorityQueue()
        {
        }

        // Have a separate Initialize method to allow
        // a. interface extraction
        // b. creating (if stored by value) or passing the queue (by value or reference) in client constructors, initializing it later
        void Initialize(TContainer* values, TCompare compare)
        {
            queue.Initialize(values->begin(), values->end(), compare);
        }

        int GetTopIndex() const
        {
            return queue.GetTopIndex();
        }

        void HandleUpdate(int valueIndex)
        {
            queue.HandleUpdate(valueIndex);
        }

        ~OrderedPriorityQueue()
        {
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(OrderedPriorityQueue);
    };
}

#endif /* Core_Headers_OrderedPriorityQueue_h */
