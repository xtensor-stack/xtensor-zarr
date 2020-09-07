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
#include "xtensor-zarr/xzarr_hierarchy.hpp"

namespace fs = ghc::filesystem;

namespace xt
{
    TEST(xzarr_hierarchy, read_array)
    {
        auto h = get_zarr_hierarchy("h_zarrita.zr3");
        zarray z = h.get_array("/arthur/dent");
        //auto a = xt::arange(5 * 10).reshape({5, 10});
        //EXPECT_EQ(a, z.get_array<double>());
    }

    TEST(xzarr_hierarchy, write_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        const char* hier_path = "h_xtensor.zr3";
        auto h = create_zarr_hierarchy(hier_path);
        auto a1 = h.create_array<double>("/arthur/dent", shape, chunk_shape, attrs);
        double v = 3.;
        a1(2, 1) = v;
        //a1.chunks().flush();
        //zarray z = h.get_array("/arthur/dent");
        //xchunked_array<> a2 = z.get_chunked_array<>();
        //EXPECT_EQ(a2(2, 1), v);
        //EXPECT_EQ(a2.attrs(), attrs);
    }
}
