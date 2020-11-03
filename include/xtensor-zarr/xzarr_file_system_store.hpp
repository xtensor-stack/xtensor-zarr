/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_FILE_SYSTEM_STORE_HPP
#define XTENSOR_ZARR_FILE_SYSTEM_STORE_HPP

#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "ghc/filesystem.hpp"
#include "xtensor-io/xio_disk_handler.hpp"

namespace fs = ghc::filesystem;

namespace xt
{
    class xzarr_file_system_stream
    {
    public:
        xzarr_file_system_stream(const std::string& path);
        operator std::string() const;
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);
        void erase();
        bool exists();

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
    };

    class xzarr_file_system_store
    {
    public:
        template <class C>
        using io_handler = xio_disk_handler<C>;

        xzarr_file_system_store(const std::string& root);
        xzarr_file_system_stream operator[](const std::string& key);
        void list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes);
        std::vector<std::string> list();
        std::vector<std::string> list_prefix(const std::string& prefix);
        void erase(const std::string& key);
        void delete_prefix(const std::string& prefix);
        void set(const std::string& key, const std::vector<char>& value);
        void set(const std::string& key, const std::string& value);
        std::string get(const std::string& key);

        xio_disk_config get_io_config();
        std::string get_root();

    private:
        std::string m_root;
    };

    /*******************************************
     * xzarr_file_system_stream implementation *
     *******************************************/

    xzarr_file_system_stream::xzarr_file_system_stream(const std::string& path)
        : m_path(path)
    {
    }

    void xzarr_file_system_stream::erase()
    {
        fs::remove(m_path);
    }

    bool xzarr_file_system_stream::exists()
    {
        std::ifstream stream(m_path);
        return stream.good();
    }

    xzarr_file_system_stream::operator std::string() const
    {
        std::ifstream stream(m_path);
        std::string bytes{std::istreambuf_iterator<char>{stream}, {}};
        return bytes;
    }

    void xzarr_file_system_stream::operator=(const std::vector<char>& value)
    {
        assign(value.data(), value.size());
    }

    void xzarr_file_system_stream::operator=(const std::string& value)
    {
        assign(value.c_str(), value.size());
    }

    void xzarr_file_system_stream::assign(const char* value, std::size_t size)
    {
        // maybe create directories
        std::size_t i = m_path.rfind('/');
        if (i != std::string::npos)
        {
            fs::path directory = m_path.substr(0, i);
            if (fs::exists(directory))
            {
                if (!fs::is_directory(directory))
                {
                    XTENSOR_THROW(std::runtime_error, "Path is not a directory: " + std::string(directory));
                }
            }
            else
            {
                fs::create_directories(directory);
            }
        }
        std::ofstream stream(m_path, std::ofstream::binary);
        stream.write(value, size);
        stream.flush();
    }

    /******************************************
     * xzarr_file_system_store implementation *
     ******************************************/

    xzarr_file_system_store::xzarr_file_system_store(const std::string& root)
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

    xzarr_file_system_stream xzarr_file_system_store::operator[](const std::string& key)
    {
        return xzarr_file_system_stream(m_root + '/' + key);
    }

    void xzarr_file_system_store::set(const std::string& key, const std::vector<char>& value)
    {
        xzarr_file_system_stream(m_root + '/' + key) = value;
    }

    void xzarr_file_system_store::set(const std::string& key, const std::string& value)
    {
        xzarr_file_system_stream(m_root + '/' + key) = value;
    }

    std::string xzarr_file_system_store::get(const std::string& key)
    {
        return std::move(xzarr_file_system_stream(m_root + '/' + key));
    }

    std::string xzarr_file_system_store::get_root()
    {
        return m_root;
    }

    xio_disk_config xzarr_file_system_store::get_io_config()
    {
        xio_disk_config c;
        return c;
    }

    void xzarr_file_system_store::list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes)
    {
        std::string path = m_root + '/' + prefix;
        for (const auto& entry: fs::directory_iterator(path))
        {
            std::string p = entry.path();
            if (fs::is_directory(p))
            {
                prefixes.push_back(p.substr(m_root.size() + 1));
            }
            else
            {
                keys.push_back(p.substr(m_root.size() + 1));
            }
        }
    }

    std::vector<std::string> xzarr_file_system_store::list()
    {
        return list_prefix("");
    }

    std::vector<std::string> xzarr_file_system_store::list_prefix(const std::string& prefix)
    {
        std::string path = m_root + '/' + prefix;
        std::vector<std::string> keys;
        for (const auto& entry: fs::recursive_directory_iterator(path))
        {
            std::string p = entry.path();
            keys.push_back(p.substr(m_root.size() + 1));
        }
        return keys;
    }

    void xzarr_file_system_store::erase(const std::string& key)
    {
        fs::remove(m_root + '/' + key);
    }

    void xzarr_file_system_store::delete_prefix(const std::string& prefix)
    {
        fs::remove_all(m_root + '/' + prefix);
    }
}

#endif
