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
    thrust::host_vector<int> offsetsKernelCall(const thrust::host_vector<Selector> &host_selectors);
    void removeKernelCall(const thrust::host_vector<Selector> &host_selectors);
    void removeKernelCallOpt(const thrust::host_vector<int> &host_offsets);
    Distribution classDistKernelCall(const thrust::host_vector<Selector> &host_selectors);
private:
    HostDataVector& m_host_data;
    DeviceDataVector& m_device_data;
    thrust::device_vector<Selector> m_device_selectors_buffer;
    const unsigned int m_classes_amount;
    const unsigned int m_alive_flag_pos;
    thrust::device_vector<int> m_device_offsets_buffer;
};
