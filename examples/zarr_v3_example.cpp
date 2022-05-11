#include "xtensor/xio.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-zarr/xzarr_array.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "zarray/zarray_zarray.hpp"

namespace xt
{
    template class XTENSOR_ZARR_API xchunked_array_factory<xzarr_file_system_store>;
}


#include <iostream>
#include <string>
#include <vector>

int main(int32_t argc, char **argv) {
  std::cout << "Hello World" << std::endl;


  std::vector<size_t> shape = {4, 4};
  std::vector<size_t> chunk_shape = {2, 2};
  auto h = xt::create_zarr_hierarchy("h_xtensor.zr3");
  xt::xzarr_create_array_options<> o;
  o.chunk_memory_layout = 'C';
  o.chunk_separator = '/';
  o.attrs = {{"question", "life"}, {"answer", 42}};
  o.chunk_pool_size = 1;
  o.fill_value = 6.6;
  {
    xt::zarray z = h.create_array("/arthur/dent", shape, chunk_shape, "<f8", o);
    auto chunked_array = z.as_chunked_array();

  }
  xt::zarray z = h.get_array("/arthur/dent");
  auto a = z.get_array<double>();
  
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      //z(i, j, static_cast<double>(i * j));
      a(2, 1) = 3.;
      }
    }

  return 0;
}
