import zarrita
import numpy as np

h = zarrita.create_hierarchy('h_zarrita.zr3')
attrs = {'question': 'life', 'answer': 42}
a = h.create_array('/arthur/dent', shape=(5, 10), dtype='float64', chunk_shape=(2, 5), compressor=None, attrs=attrs)
a[:, :] = np.arange(5 * 10).reshape(5, 10)
