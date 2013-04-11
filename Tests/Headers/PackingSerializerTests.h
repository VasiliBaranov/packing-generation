// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Headers_PackingSerializerTests_h
#define Headers_PackingSerializerTests_h

#include <boost/shared_ptr.hpp>
#include "Generation/Model/Headers/Types.h"
namespace PackingServices { class PackingSerializer; }
namespace Tests { class EndiannessProviderStub; }

namespace Tests
{
    class PackingSerializerTests
    {
    private:
        static Model::Packing particles;
        static boost::shared_ptr<PackingServices::PackingSerializer> packingSerializer;
        static boost::shared_ptr<EndiannessProviderStub> endiannessProvider;

        static const int particlesCount = 4;
    public:
        static void RunTests();
    private:
        static void SetUp();
        static void TearDown();

        static void SavePacking_InBigEndian_PackingIsDifferentWhenLoadingInLittleEndian();
        static void SavePacking_InLittleEndian_PackingIsDifferentWhenLoadingInBigEndian();
        static void SavePacking_InBigEndian_PackingIsNotChanged();
        static void SavePacking_InBigEndian_PackingIsCorrectWhenLoadedInBigEndian();
        static void SavePacking_InLittleEndian_PackingIsCorrectWhenLoadedInLittleEndian();

        static void SerializeInsertionRadii_InBigEndian_DataIsDifferentWhenLoadingInLittleEndian();
        static void SerializeInsertionRadii_InLittleEndian_DataIsDifferentWhenLoadingInBigEndian();
        static void SerializeInsertionRadii_InBigEndian_DataIsNotChanged();
        static void SerializeInsertionRadii_InBigEndian_DataIsCorrectWhenLoadedInBigEndian();
        static void SerializeInsertionRadii_InLittleEndian_DataIsCorrectWhenLoadedInLittleEndian();
    };
}

#endif /* Headers_PackingSerializerTests_h */
