#include "Selectors.h"

ISelector::ISelector(SelectorType type,float internal_value, unsigned char attr_index): 
	m_selector_type(type),m_value(internal_value), m_attr_index(attr_index)
{

}

unsigned char ISelector::index() const
{
	return m_attr_index;
}

bool ISelector::operator==(const ISelector& rhs) const
{
	return m_value == rhs.m_value && m_attr_index == rhs.m_attr_index;
}

bool ISelector::operator!=(const ISelector& rhs) const
{
	return m_value != rhs.m_value && m_attr_index != rhs.m_attr_index;
}

ISelector::SelectorType ISelector::type() const
{
	return m_selector_type;
}




