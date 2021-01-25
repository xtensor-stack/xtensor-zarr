/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"
#include "xtensor-zarr/xzarr_gdal_store.hpp"

#include "gtest/gtest.h"

namespace xt
{
    TEST(gdal, write_read_array_gdal)
    {
        xzarr_register_compressor<xzarr_gdal_store, xio_gzip_config>();

        // write array
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        std::size_t pool_size = 1;
        double fill_value = 6.6;
        xzarr_gdal_store s1("/vsimem/test.zr3");
        auto h1 = create_zarr_hierarchy(s1);
        xzarr_create_array_options<xio_gzip_config> o;
        o.chunk_memory_layout = 'C';
        o.chunk_separator = '/';
        o.attrs = attrs;
        o.chunk_pool_size = pool_size;
        o.fill_value = fill_value;
        zarray z1 = h1.create_array("/arthur/dent", shape, chunk_shape, "<f8", o);

        // read array
        xzarr_gdal_store s2("/vsimem/test.zr3");
        auto h2 = get_zarr_hierarchy(s2);
        zarray z2 = h2.get_array("/arthur/dent");
        auto ref = ones<double>({4, 4}) * fill_value;
        EXPECT_EQ(ref, z2.get_array<double>());
    }
}
