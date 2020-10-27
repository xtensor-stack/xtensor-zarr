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
        xzarr_gcs_stream(const std::string& path);
        operator std::string() const;
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
    };

    class xzarr_gcs_store
    {
    public:
        template <class C>
        using io_handler = xio_gcs_handler<C>;

        xzarr_gcs_store(const std::string& root);
        xzarr_gcs_stream operator[](const std::string& key);

        std::string get_root();

    private:
        std::string m_root;
    };

    /***********************************
     * xzarr_gcs_stream implementation *
     ***********************************/

    xzarr_gcs_stream::xzarr_gcs_stream(const std::string& path)
        : m_path(path)
    {
    }

    xzarr_gcs_stream::operator std::string() const
    {
        std::string bucket_name;
        std::string file_path;
        std::size_t i = m_path.find('/');
        bucket_name = m_path.substr(0, i);
        file_path = m_path.substr(i + 1);
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        auto reader = client.ReadObject(bucket_name, file_path);
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
        std::string bucket_name;
        std::string file_path;
        std::size_t i = m_path.find('/');
        bucket_name = m_path.substr(0, i);
        file_path = m_path.substr(i + 1);
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        auto writer = client.WriteObject(bucket_name, file_path);
        writer.write(value, size);
        writer.flush();
    }

    /**********************************
     * xzarr_gcs_store implementation *
     **********************************/

    xzarr_gcs_store::xzarr_gcs_store(const std::string& root)
        : m_root(root)
    {
        if (m_root.empty())
        {
            XTENSOR_THROW(std::runtime_error, "Root directory cannot be empty");
        }
        while (m_root.back() == '/')
        {
            m_root.pop_back();
        }
    }

    xzarr_gcs_stream xzarr_gcs_store::operator[](const std::string& key)
    {
        return xzarr_gcs_stream(m_root + '/' + key);
    }

    std::string xzarr_gcs_store::get_root()
    {
        return m_root;
    }

}

#endif
