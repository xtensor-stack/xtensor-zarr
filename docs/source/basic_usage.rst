.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Basic Usage
===========

Create a hierarchy
------------------

.. code-block:: cpp

    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"

    int main ()
    {
        // create a hierarchy on the local file system
        xt::xzarr_file_system_store store("test.zr3");
        auto h = xt::create_zarr_hierarchy(store);
    }

Open a hierarchy
----------------

.. code-block:: cpp

    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"

    int main ()
    {
        // open a hierarchy
        xt::xzarr_file_system_store store("test.zr3");
        auto h = xt::get_zarr_hierarchy(store);
    }

Create an array
---------------

.. code-block:: cpp

    #include <vector>
    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"
    #include "xtensor-io/xio_binary.hpp"

    int main ()
    {
        // open a hierarchy
        xt::xzarr_file_system_store store("test.zr3");
        auto h = xt::get_zarr_hierarchy(store);
        // create an array in the hierarchy
        nlohmann::json attrs = {{"question", "life"}, {"answer", 42}};
        using S = std::vector<std::size_t>;
        S shape = {4, 4};
        S chunk_shape = {2, 2};
        xt::zarray a = h.create_array(
            "/arthur/dent",  // path to the array in the store
            shape,  // array shape
            chunk_shape,  // chunk shape
            "<f8",  // data type, here little-endian 64-bit floating point
            'C',  // memory layout
            '/',  // chunk identifier separator
            xt::xio_binary_config(),  // compressor (here, raw binary)
            attrs,  // attributes
            10,  // chunk pool size
            0.  // fill value
        );
        // write array data
        a(2, 1) = 3.;
    }

Access an array
---------------

.. code-block:: cpp

    #include <iostream>
    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"

    int main ()
    {
        // open a hierarchy
        xt::xzarr_file_system_store store("test.zr3");
        auto h = xt::get_zarr_hierarchy(store);
        // access an array in the hierarchy
        xt::zarray a = h.get_array("/arthur/dent");
        // read array data
        std::cout << a(2, 1) << std::endl;
        // prints `3.`
        std::cout << a(2, 2) << std::endl;
        // prints `0.` (fill value)
        std::cout << a.attrs() << std::endl;
        // prints `{"answer":42,"question":"life"}`
    }

Create a group
--------------

.. code-block:: cpp

    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"

    int main ()
    {
        xt::xzarr_file_system_store store("test.zr3");
        auto h = get_zarr_hierarchy(store);
        nlohmann::json attrs = {{"heart", "gold"}, {"improbability", "infinite"}};
        // create a group
        auto g = h.create_group("/tricia/mcmillan", attrs);
    }

Explore the hierarchy
---------------------

.. code-block:: cpp

    #include <iostream>
    #include "xtensor-zarr/xzarr_hierarchy.hpp"
    #include "xtensor-zarr/xzarr_file_system_store.hpp"

    int main ()
    {
        xt::xzarr_file_system_store store("test.zr3");
        auto h = get_zarr_hierarchy(store);
        // get children at a point in the hierarchy
        std::string children = h.get_children("/").dump();
        std::cout << children << std::endl;
        // prints `{"arthur":"implicit_group","marvin":"explicit_group","tricia":"implicit_group"}`
        // view the whole hierarchy
        std::string nodes = h.get_nodes().dump();
        std::cout << nodes << std::endl;
        // prints `{"arthur":"implicit_group","arthur/dent":"array","tricia":"implicit_group","tricia/mcmillan":"explicit_group"}`
    }

Use cloud storage
-----------------

.. code-block:: cpp

    #include <iostream>
    #include "xtensor-zarr/xzarr_gcs_store.hpp"

    int main ()
    {
        // create an anonymous Google Cloud Storage client
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        xzarr_gcs_store s1("zarr-demo/v3/test.zr3", client);
        // list keys under prefix
        auto keys1 = s1.list_prefix("data/root/arthur/dent/");
        for (const auto& key: keys1)
        {
            std::cout << key << std::endl;
        }
        // prints:
        // data/root/arthur/dent/c0/0
        // data/root/arthur/dent/c0/1
        // data/root/arthur/dent/c1/0
        // data/root/arthur/dent/c1/1
        // data/root/arthur/dent/c2/0
        // data/root/arthur/dent/c2/1

        xzarr_gcs_store s2("zarr-demo/v3/test.zr3/meta/root/marvin", client);
        // list all keys
        auto keys2 = s2.list();
        for (const auto& key: keys2)
        {
            std::cout << key << std::endl;
        }
        // prints:
        // android.array.json
        // paranoid.group.json
    }
