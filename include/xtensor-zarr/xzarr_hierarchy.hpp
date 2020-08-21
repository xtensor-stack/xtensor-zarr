/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_XZARR_HIERARCHY_HPP
#define XTENSOR_ZARR_XZARR_HIERARCHY_HPP

#include <iomanip>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
#include "thirdparty/filesystem.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xtensor/xchunk_store_manager.hpp"
#include "xtensor/xfile_array.hpp"
#include "xtensor/xdisk_io_handler.hpp"
#include "xtensor-io/xblosc.hpp"
#include "xzarr_attrs.hpp"

namespace fs = ghc::filesystem;

namespace xt
{

    fs::path get_meta_path(fs::path& hier_path, const char* array_path)
    {
        while (*array_path == '/')
        {
            array_path++;
        }
        auto meta_path = hier_path / "meta" / "root" / array_path;
        fs::create_directories(meta_path.parent_path());
        return meta_path;
    }

    fs::path get_data_path(fs::path& hier_path, const char* array_path)
    {
        while (*array_path == '/')
        {
            array_path++;
        }
        auto data_path = hier_path / "data" / array_path;
        fs::create_directories(data_path);
        return data_path;
    }

    class xzarr_hierarchy
    {
    public:
        xzarr_hierarchy(const char* path): m_path(path) {};

        void create_hierarchy();

        template <class value_type, class shape_type, class io_handler = xdisk_io_handler<xblosc_config>>
        auto create_array(const char* path, shape_type shape, shape_type chunk_shape, nlohmann::json& attrs=nlohmann::json::array());

        template <class value_type, class io_handler = xdisk_io_handler<xblosc_config>>
        auto get_array(const char* path);

    private:
        fs::path m_path;
    };

    void xzarr_hierarchy::create_hierarchy()
    {
        nlohmann::json j;
        j["zarr_format"] = "https://purl.org/zarr/spec/protocol/core/3.0";
        j["metadata_encoding"] = "application/json";
        j["extensions"] = nlohmann::json::array();

        fs::create_directories(m_path);
        auto zarr_json = m_path / "zarr.json";
        std::ofstream o(zarr_json.string());
        o << std::setw(4) << j << std::endl;
    }

    template <class value_type, class shape_type, class io_handler = xdisk_io_handler<xblosc_config>>
    auto xzarr_hierarchy::create_array(const char* path, shape_type shape, shape_type chunk_shape, nlohmann::json& attrs)
    {
        auto meta_path = get_meta_path(m_path, path);
        auto meta_path_array = meta_path;
        meta_path_array += ".array";
        auto data_path = get_data_path(m_path, path);

        nlohmann::json j;
        j["shape"] = shape;
        j["chunk_grid"]["type"] = "regular";
        j["chunk_grid"]["chunk_shape"] = chunk_shape;
        j["attributes"] = attrs;
        // TODO: fix hard-coded following values:
        j["data_type"] = "<f8";
        j["chunk_memory_layout"] = "C";
        j["compressor"]["codec"] = "https://purl.org/zarr/spec/codec/blosc/1.0";
        j["compressor"]["configuration"]["clevel"] = 5;
        j["compressor"]["configuration"]["doshuffle"] = 1;
        std::ofstream stream(meta_path_array);
        stream << std::setw(4) << j << std::endl;

        xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>>, xzarr_attrs> a(shape, chunk_shape);
        a.chunks().set_directory(data_path.string().c_str());
        return a;
    }

    template <class value_type, class io_handler = xdisk_io_handler<xblosc_config>>
    auto xzarr_hierarchy::get_array(const char* path)
    {
        int i;
        auto meta_path = get_meta_path(m_path, path);
        auto meta_path_array = meta_path;
        meta_path_array += ".array";
        auto data_path = get_data_path(m_path, path);

        std::ifstream stream(meta_path_array);
        std::string s;
        stream.seekg(0, stream.end);
        s.reserve(stream.tellg());
        stream.seekg(0, stream.beg);
        s.assign((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
        auto j = nlohmann::json::parse(s);
        auto json_shape = j["shape"];
        auto json_chunk_shape = j["chunk_grid"]["chunk_shape"];
        std::vector<std::size_t> shape(json_shape.size());
        std::vector<std::size_t> chunk_shape(json_chunk_shape.size());
        i = 0;
        for (auto size: json_shape)
        {
            shape[i] = stoi(size.dump(), nullptr);
            i++;
        }
        i = 0;
        for (auto size: json_chunk_shape)
        {
            chunk_shape[i] = stoi(size.dump(), nullptr);
            i++;
        }
        xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>>, xzarr_attrs> a(shape, chunk_shape);
        a.chunks().set_directory(data_path.string().c_str());
        a.set_attrs(j["attributes"]);
        return a;
    }

    xzarr_hierarchy create_zarr_hierarchy(const char* path)
    {
        xzarr_hierarchy h(path);
        h.create_hierarchy();
        return h;
    }

    xzarr_hierarchy get_zarr_hierarchy(const char* path)
    {
        xzarr_hierarchy h(path);
        return h;
    }
}

#endif
