// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/GeometryParameters.h"

using namespace std;

namespace Core
{
    GeometryParameters::GeometryParameters()
    {
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            periodicity[i] = false;
            systemSize[i] = 0;
        }
    }

    GeometryParameters::~GeometryParameters(void)
    {

    }

    int GeometryParameters::GetNodesCount(void) const
    {
        // TODO: add a corresponding method to vector utilities, when converted to boost::array
        int nodesCount = 1;
        for (int i = 0; i < DIMENSIONS; ++i)
        {
            nodesCount *= systemSize[i];
        }
        return nodesCount;
    }

    void GeometryParameters::FillSystemSize(DiscreteSpatialVector* systemSize) const
    {
        DiscreteSpatialVector& systemSizeRef = *systemSize;
        systemSizeRef = this->systemSize;
    }

    int GeometryParameters::GetSystemSize(int dimension) const
    {
        return systemSize[dimension];
    }

    void GeometryParameters::SetSystemSize(int dimension, int size)
    {
        systemSize[dimension] = size;
    }

    bool GeometryParameters::IsPeriodicByAxis(int dimension) const
    {
        return periodicity[dimension];
    }

    void GeometryParameters::SetPeriodicity(int dimension, bool isPeriodic)
    {
        periodicity[dimension] = isPeriodic;
    }

}
