import zarr
from numcodecs import GZip
import numpy as np

z = zarr.open('h_xtensor.zr2/arthur/dent', mode='r')

assert z.chunks == (2, 2)
assert z.shape == (4, 4)
assert z.compressor == GZip(level=1)
assert z.dtype == np.dtype('float64')
assert z.attrs == {'answer': 42, 'question': 'life'}
assert z.order == 'C'
assert z.fill_value == 6.6
assert z.filters is None
