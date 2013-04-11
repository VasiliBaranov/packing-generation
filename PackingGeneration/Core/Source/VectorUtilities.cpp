// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/VectorUtilities.h"

namespace Core
{
    // Fortran arrays
    int VectorUtilities::GetColumnMajorIndex(int rowsCount, int columnsCount, int row, int column)
    {
        return rowsCount * column + row;
    }

    // C arrays
    int VectorUtilities::GetRowMajorIndex(int rowsCount, int columnsCount, int row, int column)
    {
        return columnsCount * row + column;
    }

    int VectorUtilities::GetKroneckerDelta(int i, int j)
    {
        return (i == j) ? 1 : 0;
    }
}

