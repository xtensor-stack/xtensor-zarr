/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_COMPRESSOR_HPP
#define XTENSOR_ZARR_COMPRESSOR_HPP

#include "xzarr_common.hpp"
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-io/xfile_array.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xtensor/zarray.hpp"
#include "xtl/xhalf_float.hpp"

namespace xt
{
    template <class data_type, class io_handler, class format_config>
    zarray build_chunked_array_impl(std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, format_config&& config, const nlohmann::json& config_json)
    {
        config.read_from(config_json);
        config.big_endian = (endianness == '>');
        auto a = chunked_file_array<data_type, io_handler, xzarr_index_path, xzarr_attrs>(shape, chunk_shape, path);
        a.chunks().get_index_path().set_separator(separator);
        a.chunks().configure_format(config);
        a.set_attrs(attrs);
        return zarray(std::move(a));
    }

    template <class store_type, class data_type, class format_config>
    zarray build_chunked_array_with_compressor(std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config)
    {
        using io_handler = typename store_type::template io_handler<format_config>;
        return build_chunked_array_impl<data_type, io_handler>(shape, chunk_shape, path, separator, attrs, endianness, format_config(), config);
    }

    template <class store_type, class data_type>
    class xcompressor_factory
    {
    public:

        template <class format_config>
        static void add_compressor(format_config&& c)
        {
            const char* name = c.name;
            auto fun = instance().m_builders.find(name);
            if (fun != instance().m_builders.end())
            {
                XTENSOR_THROW(std::runtime_error, "Compressor already registered: " + std::string(name));
            }
            instance().m_builders.insert(std::make_pair(name, &build_chunked_array_with_compressor<store_type, data_type, format_config>));
        }

        static zarray build(const std::string& compressor, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config)
        {
            auto fun = instance().m_builders.find(compressor);
            if (fun != instance().m_builders.end())
            {
                zarray z = (fun->second)(shape, chunk_shape, path, separator, attrs, endianness, config);
                return z;
            }
            else
            {
                XTENSOR_THROW(std::runtime_error, "Unkown compressor type: " + compressor);
            }
        }

    private:

        using self_type = xcompressor_factory;

        static self_type& instance()
        {
            static self_type instance;
            return instance;
        }

        xcompressor_factory()
        {
            using format_config = xio_binary_config;
            m_builders.insert(std::make_pair(format_config().name, &build_chunked_array_with_compressor<store_type, data_type, format_config>));
        }

        std::map<std::string, zarray (*)(std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config)> m_builders;
    };

    template <class store_type, class format_config>
    void xzarr_register_compressor()
    {
        xcompressor_factory<store_type, bool>::add_compressor(format_config());
        xcompressor_factory<store_type, int8_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int16_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int32_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int64_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint8_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint16_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint32_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint64_t>::add_compressor(format_config());
        xcompressor_factory<store_type, xtl::half_float>::add_compressor(format_config());
        xcompressor_factory<store_type, float>::add_compressor(format_config());
        xcompressor_factory<store_type, double>::add_compressor(format_config());
    }

}

#endif
