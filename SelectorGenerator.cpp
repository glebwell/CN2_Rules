#include "SelectorGenerator.h"

SelectorGenerator::SelectorGenerator()
{

}

void SelectorGenerator::store(Attribute::attr_type attr_type, float value, unsigned char attr_idx, std::vector<const Selector*> &vec_to_store)
{
    if (attr_type == Attribute::attr_type::DISCRETE)
    {
        auto it_bool_pair1 = m_selectors.insert(Selector(Selector::SelectorType::EQUAL, value, attr_idx));
        auto it_bool_pair2 = m_selectors.insert(Selector(Selector::SelectorType::NOT_EQUAL, value, attr_idx));
        vec_to_store.push_back(& (*it_bool_pair1.first) ); // get address of set element
        vec_to_store.push_back(& (*it_bool_pair2.first) );

    }
    else // continious
    {
        auto it_bool_pair1 = m_selectors.insert(Selector(Selector::SelectorType::LESS_EQUAL, value, attr_idx));
        auto it_bool_pair2 = m_selectors.insert(Selector(Selector::SelectorType::GREATER_EQUAL, value, attr_idx));
        vec_to_store.push_back(& (*it_bool_pair1.first) ); // get address of set element
        vec_to_store.push_back(& (*it_bool_pair2.first) );
    }

}
