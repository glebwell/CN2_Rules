#pragma once

#include <set>
#include "Selectors.h"

class SelectorGenerator
{
public:
    SelectorGenerator();
    void store(Attribute::attr_type attr_type, float value, unsigned char attr_idx, std::vector<const Selector *> &vec_to_store);
private:
    SelectorGenerator& operator=(const SelectorGenerator&) = delete;
    SelectorGenerator(const SelectorGenerator&) = delete;

    std::set<Selector> m_selectors;
};

