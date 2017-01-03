#include <cuda_runtime_api.h>

#include "DataContainer.cuh"
#include "DataFileReader.cuh"
#include "KernelArray.h"

#include <thrust/fill.h>
#include <thrust/remove.h>

DataContainer::DataContainer(HostDataVector& host_data, DeviceDataVector& device_data, unsigned int classes_amount, unsigned int alive_flag_position):
    m_host_data(host_data), m_device_data(device_data), m_classes_amount(classes_amount), m_alive_flag_pos(alive_flag_position),
    m_device_offsets_buffer( m_device_data.size() / (m_alive_flag_pos + 1) ) // max possible covery size
{

}

DataContainer::~DataContainer()
{

}

HostDataVector& DataContainer::getHostData() const
{
    return m_host_data;
}

DeviceDataVector& DataContainer::getDeviceData() const
{
    return m_device_data;
}

__device__ bool call_selectors(KernelArray<float>& dev_data, KernelArray<Selector>& sel_arr, unsigned int flag_position, unsigned int thread_id, unsigned int thread_flag_pos)
{
    // | A1 | A2 | ... | An | class_id | flag
    if ( thread_flag_pos < dev_data.m_size && dev_data.m_array[thread_flag_pos] == ALIVE_FLAG ) // check data is alive
    {
        unsigned int j, thread_attr_index;
        Selector* pSel;
        for (j = 0; j < sel_arr.m_size; ++j) // call all selectors
        {
            pSel = &sel_arr.m_array[j];
            thread_attr_index = pSel->m_attr_index + thread_id * (flag_position + 1);
            if ( !pSel->operator()(dev_data.m_array[ thread_attr_index ]) )
                break;
        }

        return j == sel_arr.m_size; // all selectors return true
    }
    return false;
}

__global__ void cn2_class_dist_kernel(KernelArray<float>dev_data, KernelArray<Selector> sel_arr, unsigned int flag_position, KernelArray<unsigned int> class_dist_out_res)
{
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int thr_flag_pos = (i + 1) * flag_position + i;
    bool success = call_selectors(dev_data, sel_arr, flag_position, i, thr_flag_pos);
    if ( success )
    {
        int class_index = (int) dev_data.m_array[ thr_flag_pos - 1 ];
        atomicAdd(&class_dist_out_res.m_array[ class_index ], 1);
    }

}

__global__ void cn2_offsets_kernel(KernelArray<float>dev_data, KernelArray<Selector> sel_arr, unsigned int flag_position, KernelArray<int> covered_indexes)
{
     unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
     unsigned int thr_flag_pos = (i + 1) * flag_position + i;
     bool success = call_selectors(dev_data, sel_arr, flag_position, i, thr_flag_pos);
     if ( success )
     {
         covered_indexes.m_array[i] = thr_flag_pos - flag_position;
     }

}

__global__ void cn2_mark_to_remove_kernel(KernelArray<float>dev_data, KernelArray<Selector> sel_arr, unsigned int flag_position)
{
     unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
     unsigned int thr_flag_pos = (i + 1) * flag_position + i;
     bool success = call_selectors(dev_data, sel_arr, flag_position, i, thr_flag_pos);
     if ( success )
     {
         dev_data.m_array[ thr_flag_pos ] = DEAD_FLAG;
     }
}

__global__ void cn2_mark_to_remove_kernel_opt(KernelArray<float>dev_data, KernelArray<int> offsets, unsigned int flag_position)
{
     unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
     if ( i < offsets.m_size )
     {
         int offset = offsets.m_array[ i ];
         if ( offset != -1 )
            dev_data.m_array[ offset + flag_position ] = DEAD_FLAG;
     }
}


Distribution DataContainer::classDistKernelCall(const thrust::host_vector<Selector> &host_selectors)
{
    thrust::device_vector<unsigned int> distribution_result(m_classes_amount);
    m_device_selectors_buffer = host_selectors;
    size_t objects_count = m_device_data.size() / (m_alive_flag_pos + 1);
    cn2_class_dist_kernel<<< ( objects_count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>
              (m_device_data, m_device_selectors_buffer, m_alive_flag_pos, distribution_result);

    cudaDeviceSynchronize();
    Distribution distribution_result_out = distribution_result;

    return distribution_result_out;
}

thrust::host_vector<int> DataContainer::offsetsKernelCall(const thrust::host_vector<Selector> &host_selectors)
{
    size_t max_possible_covery_size = m_device_offsets_buffer.size();
    thrust::fill(m_device_offsets_buffer.begin(), m_device_offsets_buffer.end(), -1);

    m_device_selectors_buffer = host_selectors;
    cn2_offsets_kernel<<< ( max_possible_covery_size + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>
              (m_device_data, m_device_selectors_buffer, m_alive_flag_pos, m_device_offsets_buffer);

    cudaDeviceSynchronize();
    /*
    auto device_covered_offsets_end = m_device_covered_offsets.end();
    auto device_covered_offsets_new_end = thrust::remove(m_device_covered_offsets.begin(), device_covered_offsets_end, -1);
    device_covered_offsets_new_end = m_device_covered_offsets.erase(device_covered_offsets_new_end, device_covered_offsets_end);
    */
    thrust::host_vector<int> result = m_device_offsets_buffer;
    return result;
}


void DataContainer::removeKernelCall(const thrust::host_vector<Selector> &host_selectors)
{
    m_device_selectors_buffer = host_selectors;
    size_t objects_count = m_device_data.size() / (m_alive_flag_pos + 1);
    cn2_mark_to_remove_kernel<<< ( objects_count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(m_device_data, m_device_selectors_buffer, m_alive_flag_pos);
    cudaDeviceSynchronize();
}


void DataContainer::removeKernelCallOpt(const thrust::host_vector<int> &host_offsets)
{
    m_device_offsets_buffer = host_offsets;
    size_t threads_count = host_offsets.size();
    cn2_mark_to_remove_kernel_opt<<< ( threads_count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(m_device_data, m_device_offsets_buffer, m_alive_flag_pos);
    cudaDeviceSynchronize();
}
