import zarr
from numcodecs import GZip
import numpy as np

z = zarr.open('h_zarr.zr2/arthur/dent', mode='w', shape=(5, 10), chunks=(2, 5), dtype='<f8', compressor=GZip(level=1), fill_value=5.5)
z.attrs['question'] = 'life'
z.attrs['answer'] =  42
z[:2, :5] = np.arange(2 * 5).reshape(2, 5)
