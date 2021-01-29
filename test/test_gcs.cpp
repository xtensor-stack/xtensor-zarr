/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-io/xio_blosc.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_gcs_store.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

#include "gtest/gtest.h"

namespace xt
{
    TEST(gcs, read_array_gcs)
    {
        xzarr_register_compressor<xzarr_gcs_store, xio_gzip_config>();
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        xzarr_gcs_store s("zarr-demo/v3/test.zr3", client);
        auto h = get_zarr_hierarchy(s);
        zarray z = h.get_array("/arthur/dent");
        auto a = arange(5 * 10).reshape({5, 10});
        EXPECT_EQ(a, z.get_array<int32_t>());
    }

    TEST(gcs, gcs_store)
    {
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        xzarr_gcs_store s1("zarr-demo/v3/test.zr3", client);
        auto keys1 = s1.list_prefix("data/root/arthur/dent/");
        std::string ref1[] = {"data/root/arthur/dent/c0/0",
                              "data/root/arthur/dent/c0/1",
                              "data/root/arthur/dent/c1/0",
                              "data/root/arthur/dent/c1/1",
                              "data/root/arthur/dent/c2/0",
                              "data/root/arthur/dent/c2/1"};
        for (std::size_t i = 0; i < 6; i++)
        {
            EXPECT_EQ(keys1[i], ref1[i]);
        }

        xzarr_gcs_store s2("zarr-demo/v3/test.zr3/meta/root/marvin", client);
        auto keys2 = s2.list();
        std::string ref2[] = {"android.array.json",
                              "paranoid.group.json"};
        for (std::size_t i = 0; i < 2; i++)
        {
            EXPECT_EQ(keys2[i], ref2[i]);
            // we don't have rights to erase objects in this bucket as an anonymous client
            EXPECT_THROW(s2.erase(keys2[i]), std::runtime_error);
        }
    }

    // TODO: enable test when zview is fixed
    //TEST(gcs, view)
    //{
    //    //init_zsystem();
    //    using dispatcher_type = zdispatcher_t<detail::xview_dummy_functor, 1, 1>;
    //    dispatcher_type::init();
    //    xzarr_register_compressor<xzarr_gcs_store, xio_blosc_config>();
    //    gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
    //    xzarr_gcs_store s("pangeo-data/gpm_imerg/late/chunk_time/precipitationCal", client);
    //    std::string metadata = s.get(".zarray");
    //    std::cerr << metadata << std::endl;
    //    auto h = get_zarr_hierarchy(s, "2");
    //    auto z = h.get_array("");
    //    xstrided_slice_vector sv({1000, all(), all()});
    //    zarray z2 = make_strided_view(z, sv);
    //    auto a = z2.get_array<float>();
    //}
}
