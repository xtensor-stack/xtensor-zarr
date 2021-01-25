# ![xtensor-zarr](docs/source/xtensor-zarr.svg)

[![Azure Pipelines](https://dev.azure.com/xtensor-stack/xtensor-stack/_apis/build/status/xtensor-stack.xtensor-zarr?branchName=master)](https://dev.azure.com/xtensor-stack/xtensor-stack/_build/latest?definitionId=9&branchName=master)
[![ReadTheDocs](https://readthedocs.org/projects/xtensor-zarr/badge/?version=latest)](http://xtensor-zarr.readthedocs.io/en/latest)
[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/xtensor-stack/xtensor-zarr/master?filepath=examples%2Fzarr_v3.ipynb)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Implementation of the Zarr version 3.0 core protocol based on xtensor

## Introduction

**xtensor-zarr is an early developer preview, and is not suitable for general usage yet. Features and implementation are subject to change.**

`xtensor-zarr` offers an API to create and access a Zarr (v3) hierarchy in a store (locally or in the cloud), read and write arrays (in various formats) and groups in the hierarchy, and explore the hierarchy.

## Installation

`xtensor-zarr` is a header-only library. We provide a package for the conda package manager.

```bash
conda install xtensor-zarr -c conda-forge
```

- `xtensor-zarr` depends on `xtensor` `^0.21.9`, `xtensor` `^0.8` and `nlohmann_json`.

- `google-cloud-cpp`, `aws-sdk-cpp`, `cpp-filesystem`, `zlib`, and `blosc` are optional dependencies to `xtensor-zarr`.

  - `google-cloud-cpp` is required to access a store in Google Cloud Storage.
  - `aws-sdk-cpp` is required to access a store in AWS S3.
  - `cpp-filesystem` is required to access a store on the local file system.
  - `zlib` is required for the GZip compressor.
  - `blosc` is required for the Blosc compressor.

All five libraries are available for the conda package manager.

You can also install `xtensor-zarr` from source:

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Trying it online

To try out xtensor-zarr interactively in your web browser, just click on the binder
link:

[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/xtensor-stack/xtensor-zarr/master?filepath=examples%2Fzarr_v3.ipynb)

## Documentation

To get started with using `xtensor-zarr`, check out the full documentation:

http://xtensor-zarr.readthedocs.io/

## Usage

```cpp
// create a hierarchy on the local file system
xt::xzarr_file_system_store store("test.zr3");
auto h = xt::create_zarr_hierarchy(store);

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

// read array data
std::cout << a(2, 1) << std::endl;
// prints `3.`
std::cout << a(2, 2) << std::endl;
// prints `0.` (fill value)
std::cout << a.attrs() << std::endl;
// prints `{"answer":42,"question":"life"}`

// create a group
auto g = h.create_group("/tricia/mcmillan", attrs);

// explore the hierarchy
// get children at a point in the hierarchy
std::string children = h.get_children("/").dump();
std::cout << children << std::endl;
// prints `{"arthur":"implicit_group","marvin":"explicit_group","tricia":"implicit_group"}`
// view the whole hierarchy
std::string nodes = h.get_nodes().dump();
std::cout << nodes << std::endl;
// prints `{"arthur":"implicit_group","arthur/dent":"array","tricia":"implicit_group","tricia/mcmillan":"explicit_group"}`

// use cloud storage
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
```

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
