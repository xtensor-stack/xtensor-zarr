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
        xzarr_gcs_stream(const std::string& path, const std::string& bucket, gcs::Client& client);
        operator std::string();
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
        std::string m_bucket;
        gcs::Client& m_client;
    };

    class xzarr_gcs_store
    {
    public:
        template <class C>
        using io_handler = xio_gcs_handler<C>;

        xzarr_gcs_store(const std::string& root, gcs::Client& client);
        xzarr_gcs_stream operator[](const std::string& key);
        std::vector<std::string> list();
        std::vector<std::string> list_prefix(const std::string& prefix);
        void list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes);
        void erase(const std::string& key);
        void erase_prefix(const std::string& prefix);
        void set(const std::string& key, const std::vector<char>& value);
        void set(const std::string& key, const std::string& value);
        std::string get(const std::string& key);

        xio_gcs_config get_io_config();
        std::string get_root();

    private:
        std::string m_root;
        std::string m_bucket;
        gcs::Client m_client;
    };

    /***********************************
     * xzarr_gcs_stream implementation *
     ***********************************/

    xzarr_gcs_stream::xzarr_gcs_stream(const std::string& path, const std::string& bucket, gcs::Client& client)
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

    xzarr_gcs_store::xzarr_gcs_store(const std::string& root, gcs::Client& client)
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

    void xzarr_gcs_store::set(const std::string& key, const std::vector<char>& value)
    {
        xzarr_gcs_stream(m_root + '/' + key, m_bucket, m_client) = value;
    }

    void xzarr_gcs_store::set(const std::string& key, const std::string& value)
    {
        xzarr_gcs_stream(m_root + '/' + key, m_bucket, m_client) = value;
    }

    std::string xzarr_gcs_store::get(const std::string& key)
    {
        return std::move(xzarr_gcs_stream(m_root + '/' + key, m_bucket, m_client));
    }

    void xzarr_gcs_store::list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes)
    {
        for (auto&& object_metadata: m_client.ListObjects(m_bucket, gcs::Prefix(m_root + '/' + prefix)))
        {
            if (!object_metadata)
            {
                XTENSOR_THROW(std::runtime_error, object_metadata.status().message());
            }
            auto key = object_metadata->name();
            key = key.substr(m_root.size() + 1);
            std::size_t i = key.find('/');
            if (i == std::string::npos)
            {
                keys.push_back(key);
            }
            else
            {
                key = key.substr(0, i + 1);
                if (prefixes.size() == 0)
                {
                    prefixes.push_back(key);
                }
                else
                {
                    if (prefixes[prefixes.size() - 1] != key)
                    {
                        prefixes.push_back(key);
                    }
                }
            }
        }
    }

    std::vector<std::string> xzarr_gcs_store::list()
    {
        return list_prefix("");
    }

    std::vector<std::string> xzarr_gcs_store::list_prefix(const std::string& prefix)
    {
        std::vector<std::string> keys;
        for (auto&& object_metadata: m_client.ListObjects(m_bucket, gcs::Prefix(m_root + '/' + prefix)))
        {
            if (!object_metadata)
            {
                XTENSOR_THROW(std::runtime_error, object_metadata.status().message());
            }
            auto key = object_metadata->name();
            key = key.substr(m_root.size() + 1);
            keys.push_back(key);
        }
        return keys;
    }

    void xzarr_gcs_store::erase(const std::string& key)
    {
        google::cloud::Status status = m_client.DeleteObject(m_bucket, m_root + '/' + key);
        if (!status.ok())
        {
            XTENSOR_THROW(std::runtime_error, status.message());
        }
    }

    void xzarr_gcs_store::erase_prefix(const std::string& prefix)
    {
        for (const auto& key: list_prefix(prefix))
        {
            erase(key);
        }
    }

    std::string xzarr_gcs_store::get_root()
    {
        return m_root;
    }

    xio_gcs_config xzarr_gcs_store::get_io_config()
    {
        xio_gcs_config c = {m_client, m_bucket};
        return c;
    }

}

#endif
