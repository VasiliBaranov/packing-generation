// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_EndiannessProviderStub_h
#define Headers_EndiannessProviderStub_h

#include "Core/Headers/IEndiannessProvider.h"

namespace Tests
{
    class EndiannessProviderStub : public Core::IEndiannessProvider
    {
    public:
        bool isBigEndian;
    public:
        EndiannessProviderStub();

        bool IsBigEndian() const;
    };
}

#endif /* Headers_EndiannessProviderStub_h */
