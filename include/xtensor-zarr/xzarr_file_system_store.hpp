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
#include "xtensor/xdisk_io_handler.hpp"

namespace fs = ghc::filesystem;

namespace xt
{
    class xzarr_file_system_stream
    {
    public:
        xzarr_file_system_stream(const char* path);
        operator std::vector<char>() const;
        operator std::string() const;
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
    };

    class xzarr_file_system_store
    {
    public:
        template <class C>
        using io_handler = xdisk_io_handler<C>;

        xzarr_file_system_store(const char* root);
        xzarr_file_system_stream operator[](const char* key);
        xzarr_file_system_stream operator[](const std::string& key);

        void set_root(const char* root);
        std::string get_root();

    private:
        std::string m_root;
    };

    /*******************************************
     * xzarr_file_system_stream implementation *
     *******************************************/

    xzarr_file_system_stream::xzarr_file_system_stream(const char* path)
        : m_path(path)
    {
    }

    xzarr_file_system_stream::operator std::vector<char>() const
    {
        std::ifstream stream(m_path);
        std::vector<char> bytes;
        stream.seekg(0, stream.end);
        bytes.resize(static_cast<std::size_t>(stream.tellg()));
        stream.seekg(0, stream.beg);
        bytes.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
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

    xzarr_file_system_store::xzarr_file_system_store(const char* root)
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

    xzarr_file_system_stream xzarr_file_system_store::operator[](const char* key)
    {
        std::string path = m_root + '/' + key;
        return xzarr_file_system_stream(path.c_str());
    }

    xzarr_file_system_stream xzarr_file_system_store::operator[](const std::string& key)
    {
        return (*this)[key.c_str()];
    }

    void xzarr_file_system_store::set_root(const char* root)
    {
        m_root = root;
    }

    std::string xzarr_file_system_store::get_root()
    {
        return m_root;
    }

}

#endif
