// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/EndiannessProviderStub.h"

namespace Tests
{
    EndiannessProviderStub::EndiannessProviderStub()
    {
        this->isBigEndian = false;
    }

    bool EndiannessProviderStub::IsBigEndian() const
    {
        return isBigEndian;
    }
}

