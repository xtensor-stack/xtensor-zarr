/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xtensor-io/xio_binary.hpp"
#include "xtensor-zarr/xzarr_aws_store.hpp"

#include "gtest/gtest.h"

namespace xt
{
    TEST(xzarr, aws_store)
    {
        Aws::SDKOptions options;
        Aws::InitAPI(options);

        Aws::S3::S3Client client;

        xzarr_aws_store s("elevation-tiles-prod", client);
        std::string v = s["main.js"];
        std::string ref = "/*jslint browser: true*/";

        EXPECT_EQ(v.substr(0, ref.size()), ref);

        Aws::ShutdownAPI(options);
    }

    TEST(xzarr, aws_store_list_prefix)
    {
        Aws::SDKOptions options;
        Aws::InitAPI(options);

        Aws::S3::S3Client client;

        xzarr_aws_store s("elevation-tiles-prod", client);

        auto keys = s.list_prefix("docs/");
        EXPECT_EQ(keys.size(), 9);

        Aws::ShutdownAPI(options);
    }

    TEST(xzarr, aws_store_list_dir)
    {
        Aws::SDKOptions options;
        Aws::InitAPI(options);

        Aws::S3::S3Client client;

        xzarr_aws_store s("elevation-tiles-prod", client);

        std::vector<std::string> keys;
        std::vector<std::string> prefixes;
        s.list_dir("", keys, prefixes);

        EXPECT_EQ(keys.size(), 0);
        EXPECT_EQ(prefixes[0], "docs/");
        EXPECT_EQ(prefixes[1], "geotiff/");

        Aws::ShutdownAPI(options);
    }
}
