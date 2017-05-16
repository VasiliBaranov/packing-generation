// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/PackingSerializerTests.h"

#include "Generation/PackingServices/Headers/PackingSerializer.h"
#include "Core/Headers/MemoryUtility.h"
#include "../Headers/Assert.h"
#include "../Headers/EndiannessProviderStub.h"

using namespace std;
using namespace Core;
using namespace Model;
using namespace PackingServices;

namespace Tests
{
    Model::Packing PackingSerializerTests::particles;
    boost::shared_ptr<PackingServices::PackingSerializer> PackingSerializerTests::packingSerializer;
    boost::shared_ptr<EndiannessProviderStub> PackingSerializerTests::endiannessProvider;

    void PackingSerializerTests::SetUp()
    {
        endiannessProvider.reset(new EndiannessProviderStub());
        packingSerializer.reset(new PackingSerializer(endiannessProvider.get()));

        particles.resize(particlesCount);

        const FLOAT_TYPE diameter = 1.0;
        const SpatialVector c0 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 5, 5);
        const SpatialVector c1 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.9, 5, 5);
        const SpatialVector c2 = REMOVE_LAST_DIMENSION_IF_NEEDED(5, 8, 5);
        const SpatialVector c3 = REMOVE_LAST_DIMENSION_IF_NEEDED(5.5, 8, 5);
        particles[0] = DomainParticle(0, diameter, c0);
        particles[1] = DomainParticle(1, diameter, c1);
        particles[2] = DomainParticle(2, diameter, c2);
        particles[3] = DomainParticle(3, diameter, c3);
    }

    void PackingSerializerTests::TearDown()
    {
    }

    void PackingSerializerTests::SavePacking_InBigEndian_PackingIsDifferentWhenLoadingInLittleEndian()
    {
        SetUp();

        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializePacking("file.txt", particles);

        endiannessProvider->isBigEndian = false;
        Packing loadedParticles;
        loadedParticles.resize(particlesCount);
        packingSerializer->ReadPacking("file.txt", &loadedParticles);

        Assert::AreNotEqual(loadedParticles[0].diameter, particles[0].diameter, "SavePacking_InBigEndian_PackingIsDifferentWhenLoadingInLittleEndian");

        TearDown();
    }

    void PackingSerializerTests::SavePacking_InLittleEndian_PackingIsDifferentWhenLoadingInBigEndian()
    {
        SetUp();

        Packing loadedParticles;
        loadedParticles.resize(particlesCount);

        endiannessProvider->isBigEndian = false;
        packingSerializer->SerializePacking("file.txt", particles);

        endiannessProvider->isBigEndian = true;
        packingSerializer->ReadPacking("file.txt", &loadedParticles);

        Assert::AreNotEqual(loadedParticles[0].diameter, particles[0].diameter, "SavePacking_InLittleEndian_PackingIsDifferentWhenLoadingInBigEndian");

        TearDown();
    }

    void PackingSerializerTests::SavePacking_InBigEndian_PackingIsNotChanged()
    {
        SetUp();

        FLOAT_TYPE oldValue = particles[0].diameter;
        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializePacking("file.txt", particles);

        Assert::AreEqual(oldValue, particles[0].diameter, "SavePacking_InBigEndian_PackingIsNotChanged");

        TearDown();
    }

    void PackingSerializerTests::SavePacking_InBigEndian_PackingIsCorrectWhenLoadedInBigEndian()
    {
        SetUp();
        Packing loadedParticles;
        loadedParticles.resize(particlesCount);

        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializePacking("file.txt", particles);
        packingSerializer->ReadPacking("file.txt", &loadedParticles);

        Assert::AreEqual(loadedParticles[0].diameter, particles[0].diameter, "SavePacking_InBigEndian_PackingIsCorrectWhenLoadedInBigEndian");

        TearDown();
    }

    void PackingSerializerTests::SavePacking_InLittleEndian_PackingIsCorrectWhenLoadedInLittleEndian()
    {
        SetUp();
        Packing loadedParticles;
        loadedParticles.resize(particlesCount);

        endiannessProvider->isBigEndian = false;
        packingSerializer->SerializePacking("file.txt", particles);
        packingSerializer->ReadPacking("file.txt", &loadedParticles);

        Assert::AreEqual(loadedParticles[0].diameter, particles[0].diameter, "SavePacking_InLittleEndian_PackingIsCorrectWhenLoadedInLittleEndian");

        TearDown();
    }

    void PackingSerializerTests::SerializeInsertionRadii_InBigEndian_DataIsDifferentWhenLoadingInLittleEndian()
    {
        SetUp();

        const int radiiCount = 4;
        vector<FLOAT_TYPE> insertionRadii(radiiCount);
        vector<FLOAT_TYPE> loadedInsertionRadii(radiiCount);
        insertionRadii[0] = 5.3;

        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializeInsertionRadii("file.txt", insertionRadii);

        endiannessProvider->isBigEndian = false;
        packingSerializer->ReadInsertionRadii("file.txt", &loadedInsertionRadii);

        Assert::AreNotEqual(loadedInsertionRadii[0], insertionRadii[0], "SerializeInsertionRadii_InBigEndian_DataIsDifferentWhenLoadingInLittleEndian");

        TearDown();
    }

    void PackingSerializerTests::SerializeInsertionRadii_InLittleEndian_DataIsDifferentWhenLoadingInBigEndian()
    {
        SetUp();

        const int radiiCount = 4;
        vector<FLOAT_TYPE> insertionRadii(radiiCount);
        vector<FLOAT_TYPE> loadedInsertionRadii(radiiCount);
        insertionRadii[0] = 5.3;

        endiannessProvider->isBigEndian = false;
        packingSerializer->SerializeInsertionRadii("file.txt", insertionRadii);

        endiannessProvider->isBigEndian = true;
        packingSerializer->ReadInsertionRadii("file.txt", &loadedInsertionRadii);

        Assert::AreNotEqual(loadedInsertionRadii[0], insertionRadii[0], "SerializeInsertionRadii_InLittleEndian_DataIsDifferentWhenLoadingInBigEndian");

        TearDown();
    }

    void PackingSerializerTests::SerializeInsertionRadii_InBigEndian_DataIsNotChanged()
    {
        SetUp();

        const FLOAT_TYPE oldValue = 5.3;
        const int radiiCount = 4;
        vector<FLOAT_TYPE> insertionRadii(radiiCount);
        insertionRadii[0] = oldValue;

        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializeInsertionRadii("file.txt", insertionRadii);

        Assert::AreEqual(oldValue, insertionRadii[0], "SerializeInsertionRadii_InBigEndian_PackingIsNotChanged");

        TearDown();
    }

    void PackingSerializerTests::SerializeInsertionRadii_InBigEndian_DataIsCorrectWhenLoadedInBigEndian()
    {
        SetUp();

        const int radiiCount = 4;
        FLOAT_TYPE insertionRadiiArray[radiiCount] = {5.3, 8.2, 2.1, 0.9};
        vector<FLOAT_TYPE> insertionRadii(insertionRadiiArray, insertionRadiiArray + radiiCount);
        vector<FLOAT_TYPE> loadedInsertionRadii(radiiCount);

        endiannessProvider->isBigEndian = true;
        packingSerializer->SerializeInsertionRadii("file.txt", insertionRadii);
        packingSerializer->ReadInsertionRadii("file.txt", &loadedInsertionRadii);

        Assert::AreVectorsEqual(loadedInsertionRadii, insertionRadii, "SerializeInsertionRadii_InBigEndian_DataIsCorrectWhenLoadedInBigEndian");

        TearDown();
    }

    void PackingSerializerTests::SerializeInsertionRadii_InLittleEndian_DataIsCorrectWhenLoadedInLittleEndian()
    {
        SetUp();

        const int radiiCount = 4;
        FLOAT_TYPE insertionRadiiArray[radiiCount] = {5.3, 8.2, 2.1, 0.9};
        vector<FLOAT_TYPE> insertionRadii(insertionRadiiArray, insertionRadiiArray + radiiCount);
        vector<FLOAT_TYPE> loadedInsertionRadii(radiiCount);

        endiannessProvider->isBigEndian = false;
        packingSerializer->SerializeInsertionRadii("file.txt", insertionRadii);
        packingSerializer->ReadInsertionRadii("file.txt", &loadedInsertionRadii);

        Assert::AreEqual(loadedInsertionRadii, insertionRadii, "SerializeInsertionRadii_InLittleEndian_DataIsCorrectWhenLoadedInLittleEndian");

        TearDown();
    }

    void PackingSerializerTests::RunTests()
    {
        SavePacking_InBigEndian_PackingIsDifferentWhenLoadingInLittleEndian();
        SavePacking_InLittleEndian_PackingIsDifferentWhenLoadingInBigEndian();
        SavePacking_InBigEndian_PackingIsNotChanged();
        SavePacking_InBigEndian_PackingIsCorrectWhenLoadedInBigEndian();
        SavePacking_InLittleEndian_PackingIsCorrectWhenLoadedInLittleEndian();

        SerializeInsertionRadii_InBigEndian_DataIsDifferentWhenLoadingInLittleEndian();
        SerializeInsertionRadii_InLittleEndian_DataIsDifferentWhenLoadingInBigEndian();
        SerializeInsertionRadii_InBigEndian_DataIsNotChanged();
        SerializeInsertionRadii_InBigEndian_DataIsCorrectWhenLoadedInBigEndian();
        SerializeInsertionRadii_InLittleEndian_DataIsCorrectWhenLoadedInLittleEndian();
    }
}


