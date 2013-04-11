// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_ByteUtilityTests_h
#define Headers_ByteUtilityTests_h

namespace Tests
{
    class ByteUtilityTests
    {
    public:
        static void RunTests();
    private:
        static void SwapBytes_FloatsTwice_NumbersEqual();
        static void SwapBytes_IntsTwice_NumbersEqual();
        static void SwapBytes_DoublesTwice_NumbersEqual();
    };
}

#endif /* Headers_ByteUtilityTests_h */
