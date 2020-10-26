/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <vector>

#include "xtensor/xview.hpp"
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-zarr/xzarr_gcs_store.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

#include "gtest/gtest.h"

namespace xt
{
    TEST(xzarr_hierarchy, read_array)
    {
        xzarr_register_compressor<xzarr_file_system_store, xio_gzip_config>();
        xzarr_file_system_store s("h_zarrita.zr3");
        auto h = get_zarr_hierarchy(s);
        zarray z = h.get_array("/arthur/dent");
        auto ref = arange(2 * 5).reshape({2, 5});
        auto a = z.get_array<double>();
        EXPECT_EQ(xt::view(a, xt::range(0, 2), xt::range(0, 5)), ref);
        EXPECT_EQ(a(2, 0), 5.5);
    }

    TEST(xzarr_hierarchy, write_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        std::size_t pool_size = 1;
        double fill_value = 6.6;
        xzarr_file_system_store s("h_xtensor.zr3");
        auto h = create_zarr_hierarchy(s);
        zarray z1 = h.create_array("/arthur/dent", shape, chunk_shape, "<f8", 'C', '/', xio_gzip_config(), attrs, pool_size, fill_value);
        //xchunked_array_factory<xzarr_file_system_store>::add_dtype<half_float::half>("f2");
        //auto a1 = z1.get_array<double>();
        //double v = 3.;
        //a1(2, 1) = v;
        //a1.chunks().flush();
        //zarray z2 = h.get_array("/arthur/dent");
        //xarray<double> a2 = z2.get_array<double>();
        //EXPECT_EQ(a2(2, 1), v);
        //EXPECT_EQ(a2.attrs(), attrs);
    }

    TEST(xzarr_hierarchy, read_array_gcs)
    {
        xzarr_register_compressor<xzarr_gcs_store, xio_gzip_config>();
        xzarr_gcs_store s("zarr-demo/v3/test.zr3");
        auto h = get_zarr_hierarchy(s);
        zarray z = h.get_array("/arthur/dent");
        auto a = arange(5 * 10).reshape({5, 10});
        EXPECT_EQ(a, z.get_array<int32_t>());
    }
}
