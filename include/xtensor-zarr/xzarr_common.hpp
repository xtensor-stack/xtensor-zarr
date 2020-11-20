/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_COMMON_HPP
#define XTENSOR_ZARR_COMMON_HPP

namespace xt
{
    inline std::size_t get_zarr_major(const std::string& zarr_version)
    {
        std::size_t i = zarr_version.find('.');
        std::size_t zarr_major;
        if (i == std::string::npos)
        {
            zarr_major = std::stoi(zarr_version);
        }
        else
        {
            zarr_major = std::stoi(zarr_version.substr(0, i));
        }
        if ((zarr_major < 2) || (zarr_major > 3))
        {
            XTENSOR_THROW(std::runtime_error, "Unsupported Zarr version: " + zarr_version);
        }
        return zarr_major;
    }

    inline std::string ensure_startswith_slash(const std::string& s)
    {
        if (s.front() == '/')
        {
            return s;
        }
        return '/' + s;
    }

    /********************************
     * xzarr_index_path declaration *
     ********************************/

    class xzarr_index_path
    {
    public:
        xzarr_index_path();

        void set_directory(const std::string& directory);
        void set_separator(const char separator);
        void set_zarr_version(std::size_t zarr_version);
        template <class I>
        void index_to_path(I first, I last, std::string& path);

    private:
        std::string m_directory;
        char m_separator;
        std::size_t m_zarr_version;
    };

    // xzarr_attrs is meant to serve as a base class extension for xchunked_array
    // it provides JSON attribute getter and setter methods
    class xzarr_attrs
    {
    public:
        nlohmann::json attrs();
        void set_attrs(const nlohmann::json& attrs);

    private:
        nlohmann::json m_attrs;
    };

    /******************************
     * xindex_path implementation *
     ******************************/

    xzarr_index_path::xzarr_index_path()
        : m_separator('/')
        , m_zarr_version(3)
    {
    }

    void xzarr_index_path::set_directory(const std::string& directory)
    {
        m_directory = directory;
        if (m_directory.back() != '/')
        {
            m_directory.push_back('/');
        }
    }

    void xzarr_index_path::set_separator(const char separator)
    {
        m_separator = separator;
    }

    void xzarr_index_path::set_zarr_version(std::size_t zarr_version)
    {
        m_zarr_version = zarr_version;
    }

    template <class I>
    void xzarr_index_path::index_to_path(I first, I last, std::string& path)
    {
        std::string fname;
        for (auto it = first; it != last; ++it)
        {
            if (fname.empty())
            {
                if (m_zarr_version == 3)
                {
                    fname.push_back('c');
                }
            }
            else
            {
                fname.push_back(m_separator);
            }
            fname.append(std::to_string(*it));
        }
        path = m_directory + fname;
    }

    nlohmann::json xzarr_attrs::attrs()
    {
        return m_attrs;
    }

    void xzarr_attrs::set_attrs(const nlohmann::json& attrs)
    {
        m_attrs = attrs;
    }

}

#endif
