// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/ByteUtilityTests.h"
#include <string>
#include "../Headers/Assert.h"
#include "Core/Headers/Utilities.h"

using namespace Core;

namespace Tests
{
    void ByteUtilityTests::SwapBytes_FloatsTwice_NumbersEqual()
    {
        float x = 5.3;
        float result = Utilities::DoByteSwap<float>(x);
        result = Utilities::DoByteSwap<float>(result);
        Assert::AreEqual(x, result, "SwapBytes_FloatsTwice_NumbersEqual");
    }

    void ByteUtilityTests::SwapBytes_IntsTwice_NumbersEqual()
    {
        int x = 5;
        int result = Utilities::DoByteSwap<int>(x);
        result = Utilities::DoByteSwap<int>(result);
        Assert::AreEqual(x, result, "SwapBytes_IntsTwice_NumbersEqual");
    }

    void ByteUtilityTests::SwapBytes_DoublesTwice_NumbersEqual()
    {
        double x = 5.3;
        double result = Utilities::DoByteSwap<double>(x);
        result = Utilities::DoByteSwap<double>(result);
        Assert::AreEqual(x, result, "SwapBytes_DoublesTwice_NumbersEqual");
    }

    void ByteUtilityTests::RunTests()
    {
        SwapBytes_FloatsTwice_NumbersEqual();
        SwapBytes_IntsTwice_NumbersEqual();
        SwapBytes_DoublesTwice_NumbersEqual();
    }
}



