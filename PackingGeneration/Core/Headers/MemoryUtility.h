// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_MemoryUtility_h
#define Core_Headers_MemoryUtility_h

#include <vector>
#include <boost/shared_ptr.hpp>

namespace Core
{
    // Represents a class to handle memory allocation and deallocation.
    class MemoryUtility
    {
    public:
        template<typename T>
        static T*** Allocate3DArray(size_t xsize, size_t ysize, size_t zsize)
        {
            T*** grid;
            grid = new T** [xsize];
            grid[0] = new T* [xsize * ysize];
            grid[0][0] = new T[xsize * ysize * zsize];

            for (size_t i = 1; i < xsize; ++i)
            {
                grid[i] = grid[i - 1] + ysize;
            }
            for (size_t i = 1; i < xsize * ysize; ++i)
            {
                grid[0][i] = grid[0][i - 1] + zsize;
            }
            return grid;
        }

        template<typename T>
        static void Free3DArray(T*** grid)
        {
            delete[] grid[0][0];
            delete[] grid[0];
            delete[] grid;
        }

        // Creates a linear row-major storage for the entire matrix, which is accessible by result[0], and a vector result with pointers to corresponding parts of the storage.
        template<typename T>
        static T** Allocate2DArray(size_t xsize, size_t ysize)
        {
            T** grid;
            grid = new T* [xsize];
            grid[0] = new T [xsize * ysize];

            for (size_t i = 1; i < xsize; ++i)
            {
                grid[i] = grid[i - 1] + ysize;
            }
            return grid;
        }

        template<typename T>
        static void Free2DArray(T** grid)
        {
            delete[] grid[0];
            delete[] grid;
        }

        template<class T>
        static void RemoveOwnership(const std::vector<boost::shared_ptr<T> >& ownedVector, std::vector<T*>* unownedVector)
        {
            int size = ownedVector.size();
            std::vector<T*>& unownedVectorRef = *unownedVector;
            unownedVectorRef.resize(ownedVector.size());
            for (int i = 0; i < size; ++i)
            {
                unownedVectorRef[i] = ownedVector[i].get();
            }
        }
    };
}

#endif /* Core_Headers_MemoryUtility_h */
