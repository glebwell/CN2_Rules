#pragma once
#include <string>

struct Attribute
{
	enum class attr_type : unsigned char{ DISCRETE, CONTINUOUS };
	std::string m_name;
	attr_type m_type;
};