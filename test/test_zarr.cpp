/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <vector>

#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"

#include "gtest/gtest.h"

namespace xt
{
    TEST(xzarr_hierarchy, read_array)
    {
        xzarr_file_system_store s("h_zarrita.zr3");
        auto h = get_zarr_hierarchy(s);
        zarray z = h.get_array("/arthur/dent");
        //auto a = xt::arange(5 * 10).reshape({5, 10});
        //EXPECT_EQ(a, z.get_array<double>());
    }

    TEST(xzarr_hierarchy, write_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        xzarr_file_system_store s("h_xtensor.zr3");
        auto h = create_zarr_hierarchy(s);
        zarray z1 = h.create_array("/arthur/dent", shape, chunk_shape, "<f8", xio_gzip_config(), attrs);
        //auto a1 = z1.get_array<double>();
        //double v = 3.;
        //a1(2, 1) = v;
        //a1.chunks().flush();
        //zarray z2 = h.get_array("/arthur/dent");
        //xarray<double> a2 = z2.get_array<double>();
        //EXPECT_EQ(a2(2, 1), v);
        //EXPECT_EQ(a2.attrs(), attrs);
    }
}
