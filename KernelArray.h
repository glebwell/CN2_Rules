#pragma once

#include <thrust/device_vector.h>

template <typename T>
struct KernelArray
{
    T* m_array;
    unsigned int m_size;
    KernelArray(thrust::device_vector<T>& d_vec):
        m_array(thrust::raw_pointer_cast(&d_vec[0])), m_size((unsigned int) d_vec.size()) {}
};
