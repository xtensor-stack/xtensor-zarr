.. Copyright (c) 2016, Johan Mabille, Sylvain Corlay and Wolf Vollprecht

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Build and configuration
=======================

Build the documentation
-----------------------

First install the tools required to build the documentation:

.. code::

    conda install breathe doxygen sphinx_rtd_theme -c conda-forge

You can then build the documentation:

.. code::

    cd docs
    make html

Type ``make help`` to see the list of available documentation targets.
