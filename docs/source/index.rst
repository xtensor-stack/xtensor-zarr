.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. image:: xtensor-io.svg

Implementation of the Zarr version 3.0 core protocol based on the xtensor_ C++ multi-dimensional array library.

Introduction
------------

xtensor-zarr implements Zarr (v3) for for reading and writing chunked arrays.

Enabling xtensor-zarr in your C++ libraries
-------------------------------------------

``xtensor``, ``xtensor-io`` and ``xtensor-zarr`` require a modern C++ compiler supporting C++14. The following C++ compilers are supported:

- On Windows platforms, Visual C++ 2015 Update 2, or more recent
- On Unix platforms, gcc 4.9 or a recent version of Clang

Licensing
---------

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the LICENSE file for details.

.. toctree::
   :caption: INSTALLATION
   :maxdepth: 2

   installation

.. toctree::
   :caption: USAGE
   :maxdepth: 2

   basic_usage
   xhighfive

.. toctree::
   :caption: API REFERENCE
   :maxdepth: 2

   api_reference

.. toctree::
   :caption: DEVELOPER ZONE

   releasing

.. _`numpy to xtensor cheat sheet`: http://xtensor.readthedocs.io/en/latest/numpy.html
.. _xtensor: https://github.com/QuantStack/xtensor
