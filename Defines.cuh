#pragma once

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>

using HostDataVector = thrust::host_vector<float>;
using DeviceDataVector = thrust::device_vector<float>;
using Distribution = thrust::host_vector<unsigned int>;

#define DEAD_FLAG 0.f
#define ALIVE_FLAG 1.f
#define THREADS_PER_BLOCK 512
