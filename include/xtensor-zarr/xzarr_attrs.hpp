/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_XZARR_ATTRS_HPP
#define XTENSOR_ZARR_XZARR_ATTRS_HPP

#include "nlohmann/json.hpp"
#include "xtensor/xchunked_array.hpp"

namespace xt
{
    class xzarr_attrs
    {
    public:
        nlohmann::json attrs();
        void set_attrs(nlohmann::json& attrs);

    private:
        nlohmann::json m_attrs;
    };

    nlohmann::json xzarr_attrs::attrs()
    {
        return m_attrs;
    }

    void xzarr_attrs::set_attrs(nlohmann::json& attrs)
    {
        m_attrs = attrs;
    }
}

#endif

