import ctypes as ct
import numpy as np
import numpy.ctypeslib as npct
from PIL import Image

import matplotlib.pyplot as plt



num_iter = 1000000


pic = Image.open('broiler.jpg').convert('L')

sample = np.array(pic).astype(np.int32)
sample = np.abs(sample - 255)

field = np.random.randint(1, 2, size=sample.size, dtype=np.int32).reshape(sample.shape)

if(sample.strides != field.strides):
    raise ValueError('sample.strides != field.strides')

x_0 = np.array([sample.shape[0] // 2, sample.shape[1] // 2], dtype=np.int32)

sample_ptr = sample.ctypes.data_as(ct.POINTER(ct.c_int))
field_ptr = field.ctypes.data_as(ct.POINTER(ct.c_int))
x_0_ptr = x_0.ctypes.data_as(ct.POINTER(ct.c_int))


lib = ct.CDLL('./peddlib.so')

lib.cpedd.argtypes = [ct.POINTER(ct.c_int), ct.POINTER(ct.c_int), ct.POINTER(ct.c_int),
                      ct.c_int, ct.c_int, ct.c_int, ct.c_int, ct.c_int,
                      ct.c_int]
lib.cpedd.restype = ct.c_void_p

for i in range(10000):
    lib.cpedd(sample_ptr, field_ptr, x_0_ptr,
              sample.shape[0], sample.shape[1], sample.strides[0], sample.strides[1], sample.size,
              num_iter)

    print(f'golal iter num = {i}')

    plt.imshow(field)
    plt.clim(0, field.mean() * 3)
    print(field.max(), field.sum())
    plt.savefig('result.png')

