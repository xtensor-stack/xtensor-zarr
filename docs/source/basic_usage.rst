.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Basic Usage
===========

Example : Reading and writing a Zarr store
------------------------------------------

.. code-block:: cpp

    #include <vector>
    #include <iostream>
    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"
    #include "xtensor-io/xio_binary.hpp"

    int main ()
    {
        // create a hierarchy
        xt::xzarr_file_system_store store("test.zr3");  // on-disk store
        auto h1 = xt::create_zarr_hierarchy(store);
        // create an array in the hierarchy
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        using S = std::vector<std::size_t>;
        S shape = {4, 4};
        S chunk_shape = {2, 2};
        xt::zarray a1 = h1.create_array(
            "/arthur/dent",  // path to the array in the store
            shape,  // array shape
            chunk_shape,  // chunk shape
            "<f8",  // data type, here little-endian 64-bit floating point
            '/',  // chunk identifier separator
            xt::xio_binary_config(),  // compressor (here, raw binary)
            attrs  // attributes
        );
        a1(2, 1) = 3.;
        a1.chunks().flush();
        // open a hierarchy
        auto h2 = xt::get_zarr_hierarchy(store);
        // open an array in the hierarchy
        xt::zarray a2 = h2.get_array("/arthur/dent");
        std::cout << a2(2, 1) << std::endl;
        std::cout << a2.attrs() << std::endl;
    }
