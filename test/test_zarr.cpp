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
#include "xtensor-io/xio_binary.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-zarr/xzarr_gcs_store.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

#include "gtest/gtest.h"

namespace xt
{
    namespace fs = ghc::filesystem;

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
        gcs::Client client(gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials()));
        xzarr_gcs_store s("zarr-demo/v3/test.zr3", client);
        auto h = get_zarr_hierarchy(s);
        zarray z = h.get_array("/arthur/dent");
        auto a = arange(5 * 10).reshape({5, 10});
        EXPECT_EQ(a, z.get_array<int32_t>());
    }

    TEST(xzarr_hierarchy, create_group)
    {
        xzarr_file_system_store store1("h_xtensor.zr3");
        auto h1 = get_zarr_hierarchy(store1);
        nlohmann::json attrs = {{"heart", "gold"}, {"improbability", "infinite"}};
        auto g1 = h1.create_group("/tricia/mcmillan", attrs);
        // since "/tricia/mcmillan" is a group, it should not be possible to get it as an array
        EXPECT_THROW(h1["/tricia/mcmillan"].get_array(), std::runtime_error);
    }

    TEST(xzarr_hierarchy, create_node)
    {
        xzarr_file_system_store store1("h_xtensor.zr3");
        auto h1 = get_zarr_hierarchy(store1);
        h1.create_group("/marvin");
        h1["/marvin"].create_group("paranoid");
        std::vector<size_t> shape = {5, 5};
        std::vector<size_t> chunk_shape = {3, 3};
        h1["/marvin"].create_array("android", shape, chunk_shape, "<f8", 'C', '/', xio_binary_config());
        // since "/marvin/android" is an array, it should not be possible to get it as a group
        EXPECT_THROW(h1["/marvin/android"].get_group(), std::runtime_error);
    }

    TEST(xzarr_hierarchy, explore)
    {
        xzarr_file_system_store store1("h_xtensor.zr3");
        auto h1 = get_zarr_hierarchy(store1);
        std::string children = h1.get_children("/").dump();
        std::string ref1 = "{\"arthur\":\"implicit_group\",\"marvin\":\"explicit_group\",\"tricia\":\"implicit_group\"}";
        EXPECT_EQ(children, ref1);
        children = h1.get_children("/marvin").dump();
        std::string ref2 = "{\"android\":\"array\",\"paranoid\":\"explicit_group\"}";
        EXPECT_EQ(children, ref2);

        children = h1.get_children().dump();
        EXPECT_EQ(children, ref1);
        children = h1["marvin"].get_children().dump();
        EXPECT_EQ(children, ref2);

        std::string nodes = h1.get_nodes().dump();
        EXPECT_EQ(nodes, "{\"arthur\":\"implicit_group\",\"arthur/dent\":\"array\",\"marvin\":\"explicit_group\",\"marvin/android\":\"array\",\"marvin/paranoid\":\"explicit_group\",\"tricia\":\"implicit_group\",\"tricia/mcmillan\":\"explicit_group\"}");
    }

    TEST(xzarr_hierarchy, store_erase)
    {
        xzarr_file_system_store store1("store1");
        std::string ref_data = "some data";
        store1["key1"] = ref_data;
        // check that the file has the right content
        std::ifstream stream1("store1/key1");
        std::string bytes{std::istreambuf_iterator<char>{stream1}, {}};
        EXPECT_EQ(bytes, ref_data);
        store1.erase("key1");
        // check that the file has been erased
        EXPECT_EQ(fs::exists("store1/key1"), false);
    }

    TEST(xzarr_hierarchy, store_delete_prefix)
    {
        fs::remove_all("store1");
        xzarr_file_system_store store1("store1");
        store1["path_to/key1"] = "some data";
        store1["path_to/key2"] = "some more data";
        store1["path_to/key3"] = "even more data";
        // check that the files have been created
        EXPECT_EQ(fs::exists("store1/path_to/key1"), true);
        EXPECT_EQ(fs::exists("store1/path_to/key2"), true);
        EXPECT_EQ(fs::exists("store1/path_to/key3"), true);
        store1.delete_prefix("path_to");
        // check that the directory hase been erased
        EXPECT_EQ(fs::exists("store1/path_to"), false);
    }
}
