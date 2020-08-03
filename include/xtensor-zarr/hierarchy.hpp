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

#include <iomanip>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"
#include "xtensor/xchunk_store_manager.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xtensor/xfile_array.hpp"
#include "xtensor/xdisk_io_handler.hpp"

namespace xt
{
    class hierarchy
    {
    public:
        hierarchy(const char* path): m_path(path) {}

        void create_hierarchy()
        {
            nlohmann::json j;
            j["zarr_format"] = "https://purl.org/zarr/spec/protocol/core/3.0";
            j["metadata_encoding"] = "application/json";
            j["extensions"] = nlohmann::json::array();

            auto zarr_json = m_path + "/zarr.json";
            std::ofstream o(zarr_json);
            o << std::setw(4) << j << std::endl;
        }

        template <class value_type, class shape_type, class io_handler = xdisk_io_handler<value_type>>
        auto create_array(const char* path, shape_type shape, shape_type chunk_shape)
        {
            nlohmann::json j;
            j["shape"] = shape;
            j["chunk_grid"]["type"] = "regular";
            j["chunk_grid"]["chunk_shape"] = chunk_shape;

            std::ofstream stream(path);
            stream << std::setw(4) << j << std::endl;

            xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>>> a(shape, chunk_shape);
            return a;
        }

        template <class value_type, class io_handler = xdisk_io_handler<value_type>>
        auto
        get_array(const char* path)
        {
            int i;
            std::ifstream stream(path);
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
            for (auto size: json_shape)
            {
                chunk_shape[i] = stoi(size.dump(), nullptr);
                i++;
            }
            xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>>> a(shape, chunk_shape);
            return a;
        }

    private:
        std::string m_path;
    };

    hierarchy create_hierarchy(const char* path)
    {
        hierarchy h(path);
        h.create_hierarchy();
        return h;
    }

    hierarchy get_hierarchy(const char* path)
    {
        hierarchy h(path);
        return h;
    }
}

#endif
