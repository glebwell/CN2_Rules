#include "Selectors.h"

std::string Selector::toString() const
{
    const std::vector<Attribute>& attr_ref = DataFileReader::getInstance().attributes();
    if (!attr_ref.empty())
    {
        std::string op;
        switch (m_type)
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
        std::string res = attr_ref.at(m_attr_index).m_name + " " + op + " " + std::to_string(m_value);
        return res;
    }
    else
        throw std::logic_error("Attributes set is empty");
}
