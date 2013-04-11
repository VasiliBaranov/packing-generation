// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/TestRunner.h"

#include <stdio.h>
#include "../Headers/RattlerRemovalServiceTests.h"
#include "../Headers/HcpGeneratorTests.h"
#include "../Headers/HessianServiceTests.h"
#include "../Headers/ClosestPairProviderTests.h"
#include "../Headers/ColumnMajorIndexingProviderTests.h"
#include "../Headers/ByteUtilityTests.h"
#include "../Headers/PackingSerializerTests.h"
#include "../Headers/OrderedPriorityQueueTests.h"
#include "../Headers/ParticleCollisionServiceTests.h"
#include "../Headers/VelocityServiceTests.h"
#include "../Headers/GeometryCollisionServiceTests.h"

namespace Tests
{
    void TestRunner::RunTests(void)
    {
        HcpGeneratorTests::RunTests();
        ClosestPairProviderTests::RunTests();
        ColumnMajorIndexingProviderTests::RunTests();
        ByteUtilityTests::RunTests();
        PackingSerializerTests::RunTests();
        OrderedPriorityQueueTests::RunTests();
        ParticleCollisionServiceTests::RunTests();
        GeometryCollisionServiceTests::RunTests();
        VelocityServiceTests::RunTests();
        RattlerRemovalServiceTests::RunTests();

#ifdef LAPACK_AVAILABLE
        HessianServiceTests::RunTests();
#endif

        printf("Success!");
    }
}
