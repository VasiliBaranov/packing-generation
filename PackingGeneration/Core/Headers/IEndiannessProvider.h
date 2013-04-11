// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_IEndiannessProvider_h
#define Core_Headers_IEndiannessProvider_h

namespace Core
{
    // Defines methods for determining the endianness of the system.
    // Use a separate interface for a single function to allow testing with stubs.
    class IEndiannessProvider
    {
    public:

        virtual bool IsBigEndian() const = 0;

        virtual ~IEndiannessProvider(){};
    };
}

#endif /* Core_Headers_IEndiannessProvider_h */