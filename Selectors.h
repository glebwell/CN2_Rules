#pragma once

#include <vector>
#include <memory>
#include <tuple>

#include <host_defines.h> // __host__ __device__

#include "Attribute.h"
#include "DataFileReader.h"

struct Selector
{
    enum class SelectorType : unsigned char{ DEFAULT, EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL };
    __host__ __device__ Selector(SelectorType type, float internal_value, unsigned char attr_index) : m_type(type), m_value(internal_value), m_attr_index(attr_index){}
    __host__ __device__ bool operator()(float test_value) const
    {
        switch (m_type)
        {
        case SelectorType::DEFAULT:
            return true;
        case SelectorType::EQUAL:
            return test_value == m_value;
        case SelectorType::NOT_EQUAL:
            return test_value != m_value;
        case SelectorType::LESS_EQUAL:
            return test_value <= m_value;
        case SelectorType::GREATER_EQUAL:
            return test_value >= m_value;
        default:
            return false;
        }
    }
    __host__ __device__ bool operator==(const Selector& rhs) const
    {
        return m_type == rhs.m_type && m_value == rhs.m_value && m_attr_index == rhs.m_attr_index;
    }
    __host__ __device__ bool operator!=(const Selector& rhs) const
    {
        return m_type != rhs.m_type && m_value != rhs.m_value && m_attr_index != rhs.m_attr_index;
    }

    bool operator<(const Selector& rhs) const
    {
        return std::make_tuple(m_value, m_attr_index, m_type) < std::make_tuple(rhs.m_value, rhs.m_attr_index, rhs.m_type);
    }

    std::string toString() const;


    SelectorType m_type; // type of selector
    float m_value; // compare value
    unsigned char m_attr_index; // attribute index
};

using SelectorPtr = std::shared_ptr<Selector>;
