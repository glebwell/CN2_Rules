#pragma once
#include "Selectors.h"

class SelectorFactory{
	public:
		static SelectorPtr make(ISelector::SelectorType type, float value, unsigned char attr_index);
};