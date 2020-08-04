/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_ZARRAY_HPP
#define XTENSOR_ZARR_ZARRAY_HPP

#include "nlohmann/json.hpp"
#include "xtensor/xchunked_array.hpp"

namespace xt
{
    template <class chunk_storage>
    class zarray: public xchunked_array<chunk_storage>
    {
    public:
        using xchunked_array<chunk_storage>::xchunked_array;

        nlohmann::json attrs()
        {
            return m_attrs;
        }

        void set_attrs(nlohmann::json& attrs)
        {
            m_attrs = attrs;
        }

    private:
        nlohmann::json m_attrs;
    };
}

#endif

