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
#include "xtensor-io/xio_binary.hpp"
#include "xzarr_chunked_array.hpp"

namespace xt
{
    template <class store_type>
    class xzarr_hierarchy
    {
    public:
        xzarr_hierarchy(store_type& store);

        void create_hierarchy();

        template <class shape_type, class C>
        zarray create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, char chunk_memory_layout='C', char chunk_separator='/', const C& compressor=xio_binary_config(), const nlohmann::json& attrs=nlohmann::json::array(), std::size_t chunk_pool_size=1, const nlohmann::json& fill_value=nlohmann::json());

        zarray get_array(const std::string& path, std::size_t chunk_pool_size=1);

    private:
        zarray get_zarray(const std::string& path);

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
        j["metadata_encoding"] = "https://purl.org/zarr/spec/protocol/core/3.0";
        j["metadata_key_suffix"] = ".json";
        j["extensions"] = nlohmann::json::array();

        m_store["zarr.json"] = j.dump(4);
    }

    template <class store_type>
    template <class shape_type, class C>
    zarray xzarr_hierarchy<store_type>::create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, char chunk_memory_layout, char chunk_separator, const C& compressor, const nlohmann::json& attrs, std::size_t chunk_pool_size, const nlohmann::json& fill_value)
    {
        nlohmann::json j;
        j["shape"] = shape;
        j["chunk_grid"]["type"] = "regular";
        j["chunk_grid"]["chunk_shape"] = chunk_shape;
        j["chunk_grid"]["separator"] = std::string(1, chunk_separator);
        j["attributes"] = attrs;
        j["data_type"] = dtype;
        j["chunk_memory_layout"] = std::string(1, chunk_memory_layout);
        nlohmann::json compressor_config;
        if (strcmp(compressor.name, "binary"))
        {
            j["compressor"]["codec"] = std::string("https://purl.org/zarr/spec/codec/") + compressor.name + "/1.0";
            compressor.write_to(compressor_config);
            j["compressor"]["configuration"] = compressor_config;
        }
        j["fill_value"] = fill_value;
        j["extensions"] = nlohmann::json::array();
        m_store[std::string("meta/root") + path + ".array.json"] = j.dump(4);
        std::string full_path = m_store.get_root() + "/data/root" + path;
        return xchunked_array_factory<store_type>::build(compressor.name, dtype, chunk_memory_layout, shape, chunk_shape, full_path, chunk_separator, attrs, compressor_config, chunk_pool_size, fill_value);
    }

    template <class store_type>
    zarray xzarr_hierarchy<store_type>::get_array(const std::string& path, std::size_t chunk_pool_size)
    {
        std::string s = m_store[std::string("meta/root") + path + ".array.json"];
        auto j = nlohmann::json::parse(s);
        auto json_shape = j["shape"];
        auto json_chunk_shape = j["chunk_grid"]["chunk_shape"];
        std::string dtype = j["data_type"];
        std::string chunk_memory_layout = j["chunk_memory_layout"];
        std::string compressor;
        nlohmann::json compressor_config;
        if (j.contains("compressor"))
        {
            compressor = j["compressor"]["codec"];
            std::size_t i = compressor.rfind('/');
            compressor = compressor.substr(0, i);
            i = compressor.rfind('/') + 1;
            compressor = compressor.substr(i, std::string::npos);
            compressor_config = j["compressor"]["configuration"];
        }
        else
        {
            compressor = "binary";
        }
        std::vector<std::size_t> shape(json_shape.size());
        std::vector<std::size_t> chunk_shape(json_chunk_shape.size());
        std::transform(json_shape.begin(), json_shape.end(), shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        std::transform(json_chunk_shape.begin(), json_chunk_shape.end(), chunk_shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        std::string chunk_separator = j["chunk_grid"]["separator"];
        std::string full_path = m_store.get_root() + "/data/root" + path;
        return xchunked_array_factory<store_type>::build(compressor, dtype, chunk_memory_layout[0], shape, chunk_shape, full_path, chunk_separator[0], j["attributes"], compressor_config, chunk_pool_size, j["fill_value"]);
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
