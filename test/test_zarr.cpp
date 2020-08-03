/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <vector>

#include "gtest/gtest.h"
#include "xtensor-zarr/hierarchy.hpp"

namespace xt
{
    TEST(hierarchy, load)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        auto h = create_hierarchy("path_to_array");
        auto a1 = h.create_array<double>("path_to_array/my_array", shape, chunk_shape);
        a1(2, 1) = 3.;
        auto a2 = h.get_array<double>("path_to_array/my_array");
    }
}
