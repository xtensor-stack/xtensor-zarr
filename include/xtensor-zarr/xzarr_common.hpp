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
    /********************************
     * xzarr_index_path declaration *
     ********************************/

    class xzarr_index_path
    {
    public:
        xzarr_index_path();

        void set_directory(const char* directory);
        void set_separator(const char separator);
        template <class I>
        void index_to_path(I first, I last, std::string& path);

    private:
        std::string m_directory;
        char m_separator;
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

    xzarr_index_path::xzarr_index_path(): m_separator('/')
    {
    }

    void xzarr_index_path::set_directory(const char* directory)
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

    template <class I>
    void xzarr_index_path::index_to_path(I first, I last, std::string& path)
    {
        std::string fname;
        for (auto it = first; it != last; ++it)
        {
            if (!fname.empty())
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
