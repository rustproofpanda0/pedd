import ctypes as ct
from signal import raise_signal
import numpy as np
import numpy.ctypeslib as npct
from PIL import Image

import matplotlib.pyplot as plt



num_iter = 1
# num_iter = 1


# pic = Image.open('boot.jpg').convert('L')
pic = Image.open('beer.jpg').convert('L')



# plt.imshow(np.array(pic))
# plt.show()

sample = np.array(pic).astype(np.int32)

# print(sample.shape)
# raise Exception


sample = np.abs(sample - 255)

# plt.imshow(sample)
# plt.colorbar()
# plt.show()

# raise

# field = np.zeros(sample.shape, dtype=np.int32)
field = np.random.randint(1, 2, size=sample.size, dtype=np.int32).reshape(sample.shape)

# print(sample.shape)

# print(544 / 4)

# print(sample.strides)


# raise


if(sample.strides != field.strides):
    raise ValueError('sample.strides != field.strides')

x_0 = np.array([sample.shape[0] // 2, sample.shape[1] // 2], dtype=np.int32)


sample_ptr = sample.ctypes.data_as(ct.POINTER(ct.c_int))
field_ptr = field.ctypes.data_as(ct.POINTER(ct.c_int))
x_0_ptr = x_0.ctypes.data_as(ct.POINTER(ct.c_int))



# plt.imshow(sample)
# plt.colorbar()
# plt.show()



lib = ct.CDLL('./peddlib.so')


# int *np_sample_ptr, int *np_field_ptr,
#            int x_size, int y_size, int x_stride, int y_stride, int size,
#            int num_iter

# lib.cpedd.argtypes = [ct.POINTER(ct.c_int), ct.POINTER(ct.c_int),
lib.cpedd.argtypes = [ct.POINTER(ct.c_int), ct.POINTER(ct.c_int), ct.POINTER(ct.c_int),
                      ct.c_int, ct.c_int, ct.c_int, ct.c_int, ct.c_int,
                      ct.c_int]
lib.cpedd.restype = ct.c_void_p


fig, ax = plt.subplots()
plt.ion()

s = field.sum()

fields = []

for i in range(1000):

    lib.cpedd(sample_ptr, field_ptr, x_0_ptr,
              sample.shape[0], sample.shape[1], sample.strides[0], sample.strides[1], sample.size,
              num_iter)

    print(f'golal iter num = {i}')

    fields.append(field)
    # np.save("field.npy", field)

    plt.imshow(field)
    plt.clim(0, field.mean() * 3)
    print(field.max(), field.sum(), np.abs(sample - field).mean())
    # plt.savefig('tmp.png')

    plt.pause(0.2)
    ax.clear()

# fig, ax = plt.subplots()
# plt.ion()

# for field in fields:
#     plt.imshow(field)
#     plt.pause(0.2)
#     ax.clear()
