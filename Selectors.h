#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "Attribute.h"
#include "DataFileReader.h"

class ISelector
{
public:
	enum class SelectorType : unsigned char{ DEFAULT, EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL };
	ISelector(SelectorType type, float internal_value, unsigned char attr_index);
	virtual bool operator()(float test_value) const = 0;
	virtual bool operator==(const ISelector&) const;
	virtual bool operator!=(const ISelector&) const;
	virtual std::string toString() const = 0;
	unsigned char index() const;
	SelectorType type() const;

protected:
    SelectorType m_selector_type; // type of selector
	const float m_value; // compare value
	const unsigned char m_attr_index; // attribute index
};

template <typename Functor>
class Selector : public ISelector
{
public:
	Selector(SelectorType type, float internal_value, unsigned char attr_index) : ISelector(type, internal_value, attr_index){}
	bool operator()(float test_value) const override
	{
		return m_func(test_value, m_value);
	}
	virtual std::string toString() const override
	{
		const std::vector<Attribute>& attr_ref = DataFileReader::getInstance().attributes();
		if (!attr_ref.empty())
		{
			std::string op;
			switch (m_selector_type)
			{
			case SelectorType::EQUAL:
				op = "=="; break;
			case SelectorType::NOT_EQUAL:
				op = "!="; break;
			case SelectorType::LESS_EQUAL:
				op = "<="; break;
			case SelectorType::GREATER_EQUAL:
				op = ">="; break;
			default:
				break;
			}
			return attr_ref.at(m_attr_index).m_name + " " + op + " " + std::to_string(m_value);
		}
		else
			throw std::logic_error("Attributes set is empty");
		
	}
	virtual bool operator==(const ISelector& rhs) const override
	{
		return m_selector_type == rhs.type() && ISelector::operator==(rhs);
	}

	virtual bool operator!=(const ISelector& rhs) const override
	{
		return m_selector_type != rhs.type() && ISelector::operator!=(rhs);
	}

private:
	Functor m_func;
};

/*
struct NotEqual
{
	bool operator()(float left, float right) const
	{
		return left != right;
	}
};
*/
struct AlwaysTrue
{
	bool operator()(float, float) const
	{
		return true;
	}
};

using SelectorPtr = std::shared_ptr<ISelector>;
