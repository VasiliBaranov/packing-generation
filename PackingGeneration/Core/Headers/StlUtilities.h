// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_StlUtilities_h
#define Core_Headers_StlUtilities_h

#include <algorithm>
#include <vector>
#include "VectorUtilities.h"
#include "Types.h"

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

        template<class TContainerSource, class TContainerTarget>
        static void ResizeAndCopy(const TContainerSource& source, TContainerTarget* target)
        {
            target->resize(source.size());
            std::copy(source.begin(), source.end(), target->begin());
        }

        template<class TContainerX, class TContainerY>
        static bool Equals(const TContainerX& x, const TContainerY& y)
        {
            return std::equal(x.begin(), x.end(), y.begin());
        }

        template<class TContainer>
        static size_t FindMinElementPosition(const TContainer& x)
        {
            typename TContainer::const_iterator minElementIt = std::min_element(x.begin(), x.end());
            return minElementIt - x.begin();
        }

        template<class TContainer>
        static size_t FindMaxElementPosition(const TContainer& x)
        {
            typename TContainer::const_iterator maxElementIt = std::max_element(x.begin(), x.end());
            return maxElementIt - x.begin();
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

        template<class TContainer, class TComparer>
        static void Sort(TContainer* vector, TComparer comparer)
        {
            std::sort(vector->begin(), vector->end(), comparer);
        }

        template<class T>
        static void SortPermutation(const std::vector<T>& vector, std::vector<int>* permutation)
        {
            permutation->resize(std::distance(vector.begin(), vector.end()));
            VectorUtilities::FillLinearScale(0, permutation);

            IndexesComparer<T> comparer(vector);
            StlUtilities::Sort(permutation, comparer);
        }

        template<class TContainer>
        static void RandomlyShuffle(TContainer* vector)
        {
            std::random_shuffle(vector->begin(), vector->end());
        }

        template<class T>
        static void Permute(const std::vector<T>& source, const std::vector<int>& permutation, std::vector<T>* target)
        {
            size_t length = std::distance(source.begin(), source.end());

            std::vector<T>& targetRef = *target;
            targetRef.resize(length);

            for (size_t i = 0; i < length; ++i)
            {
                targetRef[i] = source[permutation[i]];
            }
        }

        template<class TContainer>
        static void SortAndResizeToUnique(TContainer* vector)
        {
            std::sort(vector->begin(), vector->end());
            typename TContainer::iterator lastElement = std::unique(vector->begin(), vector->end());
            vector->resize(std::distance(vector->begin(), lastElement));
        }

        template<class TContainer, class TComparer>
        static void SortByNthElement(TContainer* vector, std::size_t index, TComparer comparer)
        {
            std::nth_element(vector->begin(), vector->begin() + index, vector->end(), comparer);
        }

        template<class TContainer>
        static void SortByNthElement(TContainer* vector, std::size_t index)
        {
            std::nth_element(vector->begin(), vector->begin() + index, vector->end());
        }

        template<class T>
        static void FindNthElementPermutation(const std::vector<T>& vector, std::size_t index, std::vector<int>* permutation)
        {
            permutation->resize(std::distance(vector.begin(), vector.end()));
            VectorUtilities::FillLinearScale(0, permutation);

            IndexesComparer<T> comparer(vector);
            StlUtilities::SortByNthElement(permutation, index, comparer);
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

        template<class TContainer>
        static void Remove(TContainer* vector, const CONTAINER_VALUE_TYPE(TContainer)& item)
        {
            vector->erase(std::remove(vector->begin(), vector->end(), item));
        }

        template<class TContainer>
        static void RemoveAt(TContainer* vector, int index)
        {
            vector->erase(vector->begin() + index);
        }

        template<class TContainer>
        static void Replace(TContainer* vector, const CONTAINER_VALUE_TYPE(TContainer)& oldItem, const CONTAINER_VALUE_TYPE(TContainer)& newItem)
        {
            std::replace(vector->begin(), vector->end(), oldItem, newItem);
        }

        template<class TContainer>
        static Nullable<size_t> GetLowerBoundIndex(const TContainer& vector, const CONTAINER_VALUE_TYPE(TContainer)& item)
        {
            typename TContainer::const_iterator it = std::lower_bound(vector.begin(), vector.end(), item);
            Nullable<size_t> result;
            result.hasValue = (it != vector.end());
            if (result.hasValue)
            {
                result.value = std::distance(vector.begin(), it);
            }
            return result;
        }

        template<class TX, class TY, class TGetError>
        static TX DoBinarySearch(const TX& leftPoint, const TX& rightPoint, const TY& expectedValue, const TY& minAcceptedError, const TX& minStep, TGetError getError)
        {
            TX leftPointLocal = leftPoint;
            TX rightPointLocal = rightPoint;
            TY error;
            TX middlePoint = (leftPointLocal + rightPointLocal) * 0.5;
            TX step;
            do
            {
                TX middlePoint = (leftPointLocal + rightPointLocal) * 0.5;
                step = rightPointLocal - middlePoint;
                TY currentValue = getError(middlePoint);

                if (currentValue > expectedValue)
                {
                    rightPointLocal = middlePoint;
                }
                else
                {
                    leftPointLocal = middlePoint;
                }
                error = std::abs(currentValue - expectedValue);
            } while ((error > minAcceptedError) && (step > minStep));

            return middlePoint;
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

        template<class T>
        class IndexesComparer
        {
        private:
            const std::vector<T>& values;

        public:
            IndexesComparer(const std::vector<T>& valuesArray) : values(valuesArray)
            {
            };

            bool operator()(int i, int j)
            {
                return values[i] < values[j];
            };
        };
    };
}

#endif /* Core_Headers_StlUtilities_h */
