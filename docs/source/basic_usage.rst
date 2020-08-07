.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Basic Usage
===========

Example : Reading and writing a Zarr store on disk using Blosc
--------------------------------------------------------------

.. code-block:: cpp

    #include <vector>
    #include <iostream>
    #include "xtensor-zarr/hierarchy.hpp"
    #include "xtensor/xio.hpp"

    namespace fs = ghc::filesystem;

    int main ()
    {
        // create a hierarchy
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        const char* path = "test.zr3";
        auto h = xt::create_hierarchy(path);
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        // create an array
        auto a1 = h.create_array<double>("/arthur/dent", shape, chunk_shape, attrs);
        a1(2, 1) = 3.;
        a1.chunks().flush();
        // access an array
        auto a2 = h.get_array<double>("/arthur/dent");
        std::cout << a2(2, 1) << std::endl;
        std::cout << a2.attrs() << std::endl;
    }
