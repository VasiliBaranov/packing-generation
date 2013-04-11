// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_ByteUtility_h
#define Core_Headers_ByteUtility_h

namespace Core
{
    // Didn't use the code from http://stackoverflow.com/a/3522853 (How do I convert between big-endian and little-endian values in C++?), as
    // a. may be non-portable (esp. to Power6, Bluegene)
    // b. "htonl and ntohl can't go to little endian on a big-endian platform"
    // c. some type conversions issue warnings and do not work correctly (with g++ -O3). Other errors are possible.
    class ByteUtility
    {
    public:
        template<typename T>
        static T DoByteSwap(T value)
        {
            // "value" is passed by value, so we will change the copy of the original parameter
            T* valuePtr = &value;
            DoByteSwap(reinterpret_cast<unsigned char*>(valuePtr), sizeof(value));
            return value;
        }

    private:
        static void DoByteSwap(unsigned char* b, int n)
        {
            register int i = 0;
            register int j = n - 1;
            while (i < j)
            {
              std::swap(b[j], b[i]);
              i++, j--;
            }
        }
    };
}

#endif /* Core_Headers_ByteUtility_h */
