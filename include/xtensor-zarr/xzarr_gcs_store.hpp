/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_GCS_STORE_HPP
#define XTENSOR_ZARR_GCS_STORE_HPP

#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "xtensor-io/xio_gcs_handler.hpp"

namespace xt
{
    class xzarr_gcs_stream
    {
    public:
        xzarr_gcs_stream(const std::string& path, const std::string& bucket, gcs::Client client);
        operator std::string();
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
        std::string m_bucket;
        gcs::Client m_client;
    };

    class xzarr_gcs_store
    {
    public:
        template <class C>
        using io_handler = xio_gcs_handler<C>;

        xzarr_gcs_store(const std::string& root, gcs::Client client = gcs::Client(gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        xzarr_gcs_stream operator[](const std::string& key);

        std::string get_root();

    private:
        std::string m_root;
        std::string m_bucket;
        gcs::Client m_client;
    };

    /***********************************
     * xzarr_gcs_stream implementation *
     ***********************************/

    xzarr_gcs_stream::xzarr_gcs_stream(const std::string& path, const std::string& bucket, gcs::Client client)
        : m_path(path)
        , m_bucket(bucket)
        , m_client(client)
    {
    }

    xzarr_gcs_stream::operator std::string()
    {
        auto reader = m_client.ReadObject(m_bucket, m_path);
        std::string bytes{std::istreambuf_iterator<char>{reader}, {}};
        return bytes;
    }

    void xzarr_gcs_stream::operator=(const std::vector<char>& value)
    {
        assign(value.data(), value.size());
    }

    void xzarr_gcs_stream::operator=(const std::string& value)
    {
        assign(value.c_str(), value.size());
    }

    void xzarr_gcs_stream::assign(const char* value, std::size_t size)
    {
        auto writer = m_client.WriteObject(m_bucket, m_path);
        writer.write(value, size);
        writer.flush();
    }

    /**********************************
     * xzarr_gcs_store implementation *
     **********************************/

    xzarr_gcs_store::xzarr_gcs_store(const std::string& root, gcs::Client client)
        : m_root(root)
        , m_client(client)
    {
        if (m_root.empty())
        {
            XTENSOR_THROW(std::runtime_error, "Root directory cannot be empty");
        }
        while (m_root.back() == '/')
        {
            m_root.pop_back();
        }
        std::size_t i = m_root.find('/');
        if (i == std::string::npos)
        {
            m_bucket = m_root;
            m_root = "";
        }
        else
        {
            m_bucket = m_root.substr(0, i);
            m_root = m_root.substr(i + 1);
        }
    }

    xzarr_gcs_stream xzarr_gcs_store::operator[](const std::string& key)
    {
        return xzarr_gcs_stream(m_root + '/' + key, m_bucket, m_client);
    }

    std::string xzarr_gcs_store::get_root()
    {
        return m_root;
    }

}

#endif
