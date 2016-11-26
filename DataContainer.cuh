#pragma once

#include "Defines.cuh"
#include "Selectors.h"

class DataContainer
{
public:
    DataContainer(HostDataVector& host_data, DeviceDataVector& device_data, unsigned int classes_amount, unsigned int alive_flag_position);
    ~DataContainer();
    HostDataVector& getHostData() const;
    DeviceDataVector& getDeviceData() const;
    thrust::host_vector<int> countKernelCall(const thrust::host_vector<Selector> &host_selectors, Distribution& distribution_result_out);
    void removeKernelCall(const thrust::host_vector<Selector>& host_selectors);
private:
    HostDataVector& m_host_data;
    DeviceDataVector& m_device_data;
    thrust::device_vector<Selector> m_device_selectors;
    const unsigned int m_classes_amount;
    const unsigned int m_alive_flag_pos;
    //unsigned int m_deleted_objects_count;
    thrust::device_vector<unsigned int> m_device_covered_indexes;
    unsigned int* m_device_covered_indexes_count;
};
