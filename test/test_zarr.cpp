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

namespace fs = ghc::filesystem;

namespace xt
{
    TEST(hierarchy, create)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        const char* hier_path = "test.zr3";
        auto h = create_hierarchy(hier_path);
        auto a1 = h.create_array<double>("/arthur/dent", shape, chunk_shape);
        double v = 3.;
        a1(2, 1) = v;
        a1.chunks().flush();
        auto a2 = h.get_array<double>("/arthur/dent");
        EXPECT_EQ(a2(2, 1), v);
    }
}
