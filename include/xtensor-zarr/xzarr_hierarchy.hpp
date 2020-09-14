/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_HIERARCHY_HPP
#define XTENSOR_ZARR_HIERARCHY_HPP

#include "nlohmann/json.hpp"
#include "xtensor/zarray.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xtensor/xchunk_store_manager.hpp"
#include "xtensor/xfile_array.hpp"
#include "xtensor-io/xio_binary.hpp"

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

    template <class store_type>
    class xzarr_hierarchy
    {
    public:
        template <class value_type, class io_handler>
        using tensor_type = xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>, xzarr_index_path>, xzarr_attrs>;

        xzarr_hierarchy(store_type& store);

        void create_hierarchy();

        template <class shape_type, class C>
        zarray create_array(const char* path, shape_type shape, shape_type chunk_shape, const char* dtype, const C& compressor, const nlohmann::json& attrs=nlohmann::json::array());

        zarray get_array(const char* path);

    private:
        zarray get_zarray(const char* path);

        store_type& m_store;
    };

    template <class store_type>
    xzarr_hierarchy<store_type>::xzarr_hierarchy(store_type& store)
        : m_store(store)
    {
    }

    template <class store_type>
    void xzarr_hierarchy<store_type>::create_hierarchy()
    {
        nlohmann::json j;
        j["zarr_format"] = "https://purl.org/zarr/spec/protocol/core/3.0";
        j["metadata_encoding"] = "application/json";
        j["extensions"] = nlohmann::json::array();

        m_store["zarr.json"] = j.dump(4);
    }

    template <class store_type>
    template <class shape_type, class C>
    zarray xzarr_hierarchy<store_type>::create_array(const char* path, shape_type shape, shape_type chunk_shape, const char* dtype, const C& compressor, const nlohmann::json& attrs)
    {
        nlohmann::json j;
        j["shape"] = shape;
        j["chunk_grid"]["type"] = "regular";
        j["chunk_grid"]["chunk_shape"] = chunk_shape;
        j["attributes"] = attrs;
        // TODO: fix hard-coded following values:
        j["data_type"] = dtype;
        j["chunk_memory_layout"] = "C";
        j["compressor"]["codec"] = std::string("https://purl.org/zarr/spec/codec/") + compressor.name + "/" + compressor.version;
        compressor.write(j["compressor"]["configuration"]);
        j["fill_value"] = nlohmann::json();
        j["extensions"] = nlohmann::json::array();
        m_store[std::string("meta/root") + path + ".array"] = j.dump(4);

        if (strcmp(dtype, "float64") == 0)
        {
            using io_handler = typename store_type::template io_handler<C>;
            tensor_type<double, io_handler> a(shape, chunk_shape);
            a.chunks().set_directory((m_store.get_root() + "data/root" + path).c_str());
            a.chunks().get_index_path().set_separator('.');
            a.set_attrs(attrs);
            return zarray(a);
        }
        return zarray();
    }

    template <class store_type>
    zarray xzarr_hierarchy<store_type>::get_array(const char* path)
    {
        std::vector<char> s = m_store[std::string("meta/root") + path + ".array"];
        auto j = nlohmann::json::parse(s.data());
        auto json_shape = j["shape"];
        auto json_chunk_shape = j["chunk_grid"]["chunk_shape"];
        std::string dtype = j["data_type"];
        std::vector<std::size_t> shape(json_shape.size());
        std::vector<std::size_t> chunk_shape(json_chunk_shape.size());
        std::transform(json_shape.begin(), json_shape.end(), shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        std::transform(json_chunk_shape.begin(), json_chunk_shape.end(), chunk_shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        if (dtype == "float64")
        {
            using io_handler = typename store_type::template io_handler<xio_binary_config>;
            tensor_type<double, io_handler> a(shape, chunk_shape);
            a.chunks().set_directory((m_store.get_root() + "data/root" + path).c_str());
            a.chunks().get_index_path().set_separator('.');
            a.set_attrs(j["attributes"]);
            return zarray(a);
        }
        return zarray();
    }

    template <class store_type>
    xzarr_hierarchy<store_type> create_zarr_hierarchy(store_type& store)
    {
        xzarr_hierarchy<store_type> h(store);
        h.create_hierarchy();
        return h;
    }

    template <class store_type>
    xzarr_hierarchy<store_type> get_zarr_hierarchy(store_type& store)
    {
        xzarr_hierarchy<store_type> h(store);
        return h;
    }
}

#endif
