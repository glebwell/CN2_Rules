#include <cuda_runtime_api.h>

#include "DataContainer.cuh"
#include "DataFileReader.cuh"
#include "KernelArray.h"

#include <thrust/fill.h>
#include <thrust/remove.h>

DataContainer::DataContainer(HostDataVector& host_data, DeviceDataVector& device_data, unsigned int classes_amount, unsigned int alive_flag_position):
    m_host_data(host_data), m_device_data(device_data), m_classes_amount(classes_amount), m_alive_flag_pos(alive_flag_position),
    m_device_covered_indexes( m_device_data.size() / (m_alive_flag_pos + 1) ) // max possible covery size
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

__global__ void cn2_count_kernel(KernelArray<float>dev_data, KernelArray<Selector> sel_arr, unsigned int flag_position, KernelArray<unsigned int> res_arr,
                                 KernelArray<int> covered_indexes/*, KernelArray<unsigned int> covered_indexes_count*/)
{
     unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

     // | A1 | A2 | ... | An | class_id | flag
     unsigned int thr_flag_pos = (i + 1) * flag_position + i;
     if ( thr_flag_pos < dev_data.m_size && dev_data.m_array[thr_flag_pos] == ALIVE_FLAG ) // check data is alive
     {
         unsigned int j, thr_attr_index;
         int class_index;
         Selector* pSel;
         for (j = 0; j < sel_arr.m_size; ++j) // call all selectors
         {
             pSel = &sel_arr.m_array[j];
             thr_attr_index = pSel->m_attr_index + i * (flag_position + 1);
             if ( !pSel->operator()(dev_data.m_array[ thr_attr_index ]) )
                 break;

         }

         if ( j == sel_arr.m_size ) // all selectors return true
         {

             // increment counter in class position
             class_index = (int) dev_data.m_array[ thr_flag_pos - 1 ];
             atomicAdd(&res_arr.m_array[ class_index ], 1);
             covered_indexes.m_array[i] = thr_flag_pos - flag_position;
         }
     }
}

__global__ void cn2_mark_to_remove_kernel(KernelArray<float>dev_data, KernelArray<Selector> sel_arr, unsigned int flag_position)
{
     unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

     // | A1 | A2 | ... | An | class_id | flag
     unsigned int thr_flag_pos = (i + 1) * flag_position + i;
     if ( thr_flag_pos < dev_data.m_size && dev_data.m_array[thr_flag_pos] == ALIVE_FLAG ) // check data is alive
     {
         unsigned int j, thr_attr_index;

         Selector* pSel;
         for (j = 0; j < sel_arr.m_size; ++j) // call all selectors
         {
             pSel = &sel_arr.m_array[j];
             thr_attr_index = pSel->m_attr_index + i * (flag_position + 1);
             if ( !pSel->operator()(dev_data.m_array[ thr_attr_index ]) )
                 break;
         }

         if ( j == sel_arr.m_size ) // all selectors return true
         {
             // invalidate flag position
             dev_data.m_array[ thr_flag_pos ] = DEAD_FLAG;
         }

     }
}

thrust::host_vector<int> DataContainer::countKernelCall(const thrust::host_vector<Selector>& host_selectors, Distribution& distribution_result_out )
{
    thrust::device_vector<unsigned int> distribution_result(m_classes_amount);
    size_t max_possible_covery_size = m_device_covered_indexes.size();
    thrust::fill(m_device_covered_indexes.begin(), m_device_covered_indexes.end(), -1);
    //thrust::device_vector<int> device_covered_indexes(max_possible_covery_size, -1);

    m_device_selectors = host_selectors;
    cn2_count_kernel<<< ( max_possible_covery_size + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>
              (m_device_data, m_device_selectors, m_alive_flag_pos, distribution_result, m_device_covered_indexes);

    cudaDeviceSynchronize();
    distribution_result_out = distribution_result;
    //m_device_covered_indexes_end = thrust::remove(m_device_covered_indexes.begin(), m_device_covered_indexes.end(), -1);

    return thrust::host_vector<int>(m_device_covered_indexes.begin(), m_device_covered_indexes.end());
}

void DataContainer::removeKernelCall(const thrust::host_vector<Selector>& host_selectors)
{
    m_device_selectors = host_selectors;
    size_t objects_count = m_device_data.size() / (m_alive_flag_pos + 1);
    cn2_mark_to_remove_kernel<<< ( objects_count + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(m_device_data, m_device_selectors, m_alive_flag_pos);
    cudaDeviceSynchronize();
}
