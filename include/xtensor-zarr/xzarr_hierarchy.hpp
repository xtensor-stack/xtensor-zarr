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
#include "ghc/filesystem.hpp"
#include "xtensor/zarray.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xtensor/xchunk_store_manager.hpp"
#include "xtensor/xfile_array.hpp"
#include "xtensor/xdisk_io_handler.hpp"
#include "xtensor-io/xblosc.hpp"

namespace fs = ghc::filesystem;

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
        void set_attrs(nlohmann::json& attrs);

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

        // maybe create directories
        std::size_t i = path.rfind('/');
        if (i != std::string::npos)
        {
            fs::path directory = path.substr(0, i);
            if (fs::exists(directory))
            {
                if (!fs::is_directory(directory))
                {
                    XTENSOR_THROW(std::runtime_error, "in zarr index to path transformation, this path is not a directory: " + std::string(directory));
                }
            }
            else
            {
                fs::create_directories(directory);
            }
        }
    }

    nlohmann::json xzarr_attrs::attrs()
    {
        return m_attrs;
    }

    void xzarr_attrs::set_attrs(nlohmann::json& attrs)
    {
        m_attrs = attrs;
    }

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
        template <class value_type, class io_handler>
        using tensor_type = xchunked_array<xchunk_store_manager<xfile_array<value_type, io_handler>, xzarr_index_path>, xzarr_attrs>;

        xzarr_hierarchy(const char* path): m_path(path) {};

        void create_hierarchy();

        template <class value_type, class shape_type, class io_handler = xdisk_io_handler<xblosc_config>>
        tensor_type<value_type, io_handler> create_array(const char* path, shape_type shape, shape_type chunk_shape, nlohmann::json& attrs=nlohmann::json::array());

        zarray get_array(const char* path);

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
    xzarr_hierarchy::tensor_type<value_type, io_handler> xzarr_hierarchy::create_array(const char* path, shape_type shape, shape_type chunk_shape, nlohmann::json& attrs)
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
        std::ofstream stream(meta_path_array);
        stream << std::setw(4) << j << std::endl;

        tensor_type<value_type, io_handler> a(shape, chunk_shape);
        a.chunks().set_directory(data_path.string().c_str());
        a.chunks().get_index_path().set_separator('.');
        return a;
    }

    zarray xzarr_hierarchy::get_array(const char* path)
    {
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
        std::transform(json_shape.begin(), json_shape.end(), shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        std::transform(json_chunk_shape.begin(), json_chunk_shape.end(), chunk_shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        if (true)  // TODO: instantiate the right tensor_type depending on data type, compressor...
        {
            tensor_type<double, xdisk_io_handler<xblosc_config>> a(shape, chunk_shape);
            a.chunks().set_directory(data_path.string().c_str());
            a.chunks().get_index_path().set_separator('.');
            a.set_attrs(j["attributes"]);
            return zarray(a);
        }
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
