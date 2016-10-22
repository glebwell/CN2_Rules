#include "SelectorFactory.h"

using SEqual = Selector<std::equal_to<float>>;
using SNotEqual = Selector<std::not_equal_to<float>>;
using SGreaterEqual = Selector<std::greater_equal<float>>;
using SLessEqual = Selector<std::less_equal<float>>;
using SDefault = Selector<AlwaysTrue>;

SelectorPtr SelectorFactory::make(ISelector::SelectorType type, float v, unsigned char i)
{
	switch (type)
	{
	case ISelector::SelectorType::DEFAULT:
		return std::make_shared<SDefault>(type, 0.f, 0);
		break;
	case ISelector::SelectorType::EQUAL:
		return std::make_shared<SEqual>(type, v, i);
	case ISelector::SelectorType::NOT_EQUAL:
		return std::make_shared<SNotEqual>(type, v, i);
		break;
	case ISelector::SelectorType::LESS_EQUAL:
		return std::make_shared<SLessEqual>(type, v, i);
		break;
	case ISelector::SelectorType::GREATER_EQUAL:
		return std::make_shared<SGreaterEqual>(type, v, i);
		break;
	default:
		throw std::logic_error("Bad type of selector");
	}
}
