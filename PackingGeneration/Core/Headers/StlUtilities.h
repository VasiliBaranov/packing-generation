// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_StlUtilities_h
#define Core_Headers_StlUtilities_h

#include <algorithm>
#include <vector>

// Provides wrapper methods over stl that accept containers directly and operate over entire containers (e.g., sort the entire container).
// In most cases you deal with entire containers, not a subrange of values.
namespace Core
{
    #define CONTAINER_VALUE_TYPE(TContainer)typename TContainer::value_type
    class StlUtilities
    {
    public:
        template<class TContainerSource, class TContainerTarget>
        static void Copy(const TContainerSource& source, TContainerTarget* target)
        {
            std::copy(source.begin(), source.end(), target->begin());
        }

        template<class TContainerX, class TContainerY>
        static bool Equals(const TContainerX& x, const TContainerY& y)
        {
            return std::equal(x.begin(), x.end(), y.begin());
        }

        template<class TIterator>
        static bool Exists(TIterator begin, TIterator end, const typename std::iterator_traits<TIterator>::value_type& item)
        {
            return std::find(begin, end, item) != end;
        }

        template<class TContainer>
        static bool Exists(const TContainer& vector, const CONTAINER_VALUE_TYPE(TContainer)& item)
        {
            return Exists(vector.begin(), vector.end(), item);
        }

        template<class TContainerTo, class TContainerValue>
        static void Append(const TContainerValue& value, TContainerTo* to)
        {
            to->insert(to->end(), value.begin(), value.end());
        }

        template<class TContainer>
        static void Sort(TContainer* vector)
        {
            std::sort(vector->begin(), vector->end());
        }

        template<class TContainer>
        static void ResizeToUnique(TContainer* vector)
        {
            typename TContainer::iterator lastElement = std::unique(vector->begin(), vector->end());
            vector->resize(std::distance(vector->begin(), lastElement));
        }

        template<class TContainer, class TComparer>
        static void SortByNthElement(TContainer* vector, std::size_t index, TComparer comparer)
        {
            std::nth_element(vector->begin(), vector->begin() + index, vector->end(), comparer);
        }

        template<class TContainerX, class TContainerY, class TContainerResult>
        static void FindSetIntersection(const TContainerX& x, const TContainerY& y, TContainerResult* result)
        {
            result->resize(std::max(x.size(), y.size()));
            typename TContainerResult::iterator intersectionEnd = std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), result->begin());
            result->resize(std::distance(result->begin(), intersectionEnd));
        }

        template<class TBaseClass, class TClass>
        static TClass* FindObject(const std::vector<TBaseClass*>& vector)
        {
            FindObjectFunctor<TBaseClass, TClass> findObjectFunctor;
            typename std::vector<TBaseClass*>::const_iterator objectIterator = std::find_if(vector.begin(), vector.end(), findObjectFunctor);
            if (objectIterator == vector.end())
            {
                return NULL;
            }

            TBaseClass* baseObject = *objectIterator;
            TClass* object = dynamic_cast<TClass*>(baseObject);
            return object;
        }

        template<class TBaseClass, class TClass>
        static TClass* RemoveObject(std::vector<TBaseClass*>* vector)
        {
            FindObjectFunctor<TBaseClass, TClass> findObjectFunctor;
            typename std::vector<TBaseClass*>::iterator objectIterator = std::find_if(vector->begin(), vector->end(), findObjectFunctor);
            if (objectIterator == vector->end())
            {
                return NULL;
            }

            TBaseClass* baseObject = *objectIterator;
            TClass* object = dynamic_cast<TClass*>(baseObject);
            if (object != NULL)
            {
                vector->erase(objectIterator);
            }
            return object;
        }

        // Quickly remove in O(1): copy the last element to the index of the deleted element, pop the last element.
        template<class TContainer>
        static void QuicklyRemoveAt(TContainer* vector, std::size_t index)
        {
            TContainer& vectorRef = *vector;
            vectorRef[index] = vectorRef.back();
            vectorRef.pop_back();
        }

        // Quickly remove in O(1): copy the last element to the index of the deleted element, pop the last element.
        template<class TContainer>
        static void QuicklyRemove(TContainer* vector, const CONTAINER_VALUE_TYPE(TContainer)& item)
        {
            typedef CONTAINER_VALUE_TYPE(TContainer) TItemType;
            typename TContainer::iterator it = std::find(vector->begin(), vector->end(), item);
            if (it != vector->end())
            {
                TItemType& itemRef = *it;
                itemRef = vector->back();
                vector->pop_back();
            }
        }

    private:
        template<class TBaseClass, class TClass, class TIterator>
        static TClass* FindObject(const std::vector<TBaseClass*>& vector, TIterator* iterator)
        {
            FindObjectFunctor<TBaseClass, TClass> findObjectFunctor;
            TIterator objectIterator = std::find_if(vector.begin(), vector.end(), findObjectFunctor);
            if (objectIterator == vector.end())
            {
                return NULL;
            }

            TBaseClass* baseObject = *objectIterator;
            TClass* object = dynamic_cast<TClass*>(baseObject);
            return object;
        }

    private:
        template<class TBaseClass, class TClassToSearch>
        class FindObjectFunctor
        {
        public:
            bool operator()(TBaseClass* element)
            {
                TClassToSearch* object = dynamic_cast<TClassToSearch*>(element);
                return (object != NULL);
            }
        };

    };
}

#endif /* Core_Headers_StlUtilities_h */
