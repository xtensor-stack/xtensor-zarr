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
#include "zarray/zarray.hpp"
#include "xzarr_node.hpp"
#include "xzarr_array.hpp"
#include "xzarr_group.hpp"
#include "xzarr_common.hpp"

namespace xt
{
    /**
     * @class xzarr_hierarchy
     * @brief Zarr hierarchy handler.
     *
     * The xzarr_hierarchy class implements a handler for creating and accessing
     * a hierarchy, an array or a group, as well as exploring the hierarchy.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     * @sa zarray, xzarr_group, xzarr_node
     */
    template <class store_type>
    class xzarr_hierarchy
    {
    public:
        xzarr_hierarchy(store_type& store, const std::string& zarr_version = "3");

        void create_hierarchy();

        template <class shape_type, class C>
        zarray create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, char chunk_memory_layout='C', char chunk_separator='/', const C& compressor=xio_binary_config(), const nlohmann::json& attrs=nlohmann::json::object(), std::size_t chunk_pool_size=1, const nlohmann::json& fill_value=nlohmann::json());

        zarray get_array(const std::string& path, std::size_t chunk_pool_size=1);

        xzarr_group<store_type> create_group(const std::string& path, const nlohmann::json& attrs=nlohmann::json::object(), const nlohmann::json& extensions=nlohmann::json::array());

        xzarr_node<store_type> operator[](const std::string& path);

        nlohmann::json get_children(const std::string& path="/");
        nlohmann::json get_nodes(const std::string& path="/");

    private:
        store_type& m_store;
        std::string m_zarr_version;
    };

    /**********************************
     * xzarr_hierarchy implementation *
     **********************************/

    template <class store_type>
    xzarr_hierarchy<store_type>::xzarr_hierarchy(store_type& store, const std::string& zarr_version)
        : m_store(store)
        , m_zarr_version(zarr_version)
    {
    }

    template <class store_type>
    void xzarr_hierarchy<store_type>::create_hierarchy()
    {
        if (get_zarr_major(m_zarr_version) == 3)
        {
            nlohmann::json j;
            j["zarr_format"] = "https://purl.org/zarr/spec/protocol/core/3.0";
            j["metadata_encoding"] = "https://purl.org/zarr/spec/protocol/core/3.0";
            j["metadata_key_suffix"] = ".json";
            j["extensions"] = nlohmann::json::array();

            m_store["zarr.json"] = j.dump(4);
        }
    }

    template <class store_type>
    template <class shape_type, class C>
    zarray xzarr_hierarchy<store_type>::create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, char chunk_memory_layout, char chunk_separator, const C& compressor, const nlohmann::json& attrs, std::size_t chunk_pool_size, const nlohmann::json& fill_value)
    {
        return create_zarr_array(m_store, path, shape, chunk_shape, dtype, chunk_memory_layout, chunk_separator, compressor, attrs, chunk_pool_size, fill_value, m_zarr_version);
    }

    template <class store_type>
    zarray xzarr_hierarchy<store_type>::get_array(const std::string& path, std::size_t chunk_pool_size)
    {
        return get_zarr_array(m_store, path, chunk_pool_size, m_zarr_version);
    }

    template <class store_type>
    xzarr_group<store_type> xzarr_hierarchy<store_type>::create_group(const std::string& path, const nlohmann::json& attrs, const nlohmann::json& extensions)
    {
        xzarr_group<store_type> g(m_store, path);
        return g.create_group(attrs, extensions);
    }

    template <class store_type>
    xzarr_node<store_type> xzarr_hierarchy<store_type>::operator[](const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path);
    }

    template <class store_type>
    nlohmann::json xzarr_hierarchy<store_type>::get_children(const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path).get_children();
    }

    template <class store_type>
    nlohmann::json xzarr_hierarchy<store_type>::get_nodes(const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path).get_nodes();
    }

    /************************************
     * zarr hierarchy factory functions *
     ************************************/

    /**
     * Creates a Zarr hierarchy.
     * This function creates a hierarchy in a store and returns a ``xzarr_hierarchy`` handler to it.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     *
     * @param store The hierarchy store
     *
     * @return returns a ``xzarr_hierarchy`` handler.
     */
    template <class store_type>
    xzarr_hierarchy<store_type> create_zarr_hierarchy(store_type& store, const std::string& zarr_version = "3")
    {
        xzarr_hierarchy<store_type> h(store, zarr_version);
        h.create_hierarchy();
        return h;
    }

    /**
     * Accesses a Zarr hierarchy.
     * This function returns a ``xzarr_hierarchy`` handler to a hierarchy in a given store.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     *
     * @param store The hierarchy store
     * @param zarr_version The version of the Zarr specification for the store
     *
     * @return returns a ``xzarr_hierarchy`` handler.
     */
    template <class store_type>
    xzarr_hierarchy<store_type> get_zarr_hierarchy(store_type& store, const std::string& zarr_version = "0")
    {
        std::string zarr_ver;
        if (zarr_version == "0")
        {
            std::vector<std::string> keys;
            std::vector<std::string> prefixes;
            store.list_dir("/", keys, prefixes);
            if (std::count(keys.begin(), keys.end(), "zarr.json"))
            {
                zarr_ver = "3";
            }
            else
            {
                zarr_ver = "2";
            }
        }
        else
        {
            zarr_ver = zarr_version;
        }
        xzarr_hierarchy<store_type> h(store, zarr_ver);
        return h;
    }
}

#endif
