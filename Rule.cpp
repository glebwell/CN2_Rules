#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include "Rule.h"

#include "DataContainer.cuh"

GuardianValidator Rule::m_validator;

Rule::Rule(RulePtr r) : m_parent_rule(r), m_covering(0), m_quality(0), m_target_class(255), m_offsets_is_calculated(false)
{
	if (m_parent_rule) // copy selectors from parent rule
	{
		m_selectors = r->selectors();
	}
}

void Rule::filterAndStore(unsigned char target_class)
{
    m_target_class = target_class; // init class value
    m_rule_dist = DataFileReader::getInstance().distribution();
    //m_rule_dist_check.assign(m_rule_dist.cbegin(), m_rule_dist.cend());
    m_covering = std::accumulate(m_rule_dist.cbegin(), m_rule_dist.cend(), 0u);
}

void Rule::filterAndStore(DataContainer &data, unsigned char target_class)
{
    m_target_class = target_class;
    m_rule_dist = data.classDistKernelCall(m_selectors);
    /*m_covered_offsets = data.offsetsKernelCall(m_selectors);*/

    //m_rule_dist_check.assign(m_rule_dist.cbegin(), m_rule_dist.cend());
    //m_covered_offsets_check.assign(m_covered_offsets.cbegin(), m_covered_offsets.cend());
    m_covering = std::accumulate(m_rule_dist.cbegin(), m_rule_dist.cend(), 0u);
}

bool Rule::testRule(const std::vector<float>& example) const
{
    //TODO: this method
    return true;
    //return applySelectors(example);
}

void Rule::doEvaluate()
{
	//
	// Laplace evaluation
	//

	size_t k = m_rule_dist.size();
	size_t class_dist;
	if (m_target_class != 255)
		class_dist = m_rule_dist[m_target_class];
	else // class is undefined
		class_dist = *std::max_element(m_rule_dist.cbegin(), m_rule_dist.cend());

	m_quality = (class_dist + 1) / (float)(m_covering + k);

}

bool Rule::isValid() const
{
	return m_validator(*this);
}

void Rule::addSelector(const Selector* s)
{
    if (s)
    {
       m_selectors.push_back(*s);
       //m_selectors_check.push_back(*s);
    }

}

unsigned char Rule::targetClass() const
{
	return m_target_class;
}

size_t Rule::coveredExamplesCount() const
{
	return m_covering;
}

const CoveryOffsets& Rule::coveryOffsets(DataContainer &data)
{
    if ( !m_offsets_is_calculated )
    {
        m_covered_offsets = data.offsetsKernelCall(m_selectors);
        m_offsets_is_calculated = true;
    }
    return m_covered_offsets;
}


size_t Rule::length() const
{
	return m_selectors.size();
}

const Distribution& Rule::distribution() const
{
	return m_rule_dist;
}

std::shared_ptr<Rule> Rule::parent() const
{
	return m_parent_rule;
}


bool Rule::operator==(const Rule& rhs) const
{
	return m_quality == rhs.m_quality
		&& m_covering == rhs.m_covering
		&& compareSelectors(rhs);
}

bool Rule::compareSelectors(const Rule& other) const
{
	if (m_selectors.size() != other.m_selectors.size())
		return false;
	else
	{
		for (size_t i = 0; i < m_selectors.size(); ++i)
		{
            if (m_selectors[i] != other.m_selectors[i])
				return false;
		}
	}

	return true;
}

bool Rule::operator!=(const Rule& rhs) const
{
	return m_quality != rhs.m_quality 
        && m_covering != rhs.m_covering && m_covered_offsets != rhs.m_covered_offsets;
}

bool Rule::operator<(const Rule& rhs) const
{
	if (m_quality < rhs.m_quality)
		return true;
	else if (m_quality > rhs.m_quality)
		return false;
	else // rule with less length is better
		return length() > rhs.length();
}

bool Rule::operator>(const Rule& rhs) const
{
	if (m_quality > rhs.m_quality)
		return true;
	else if (m_quality < rhs.m_quality)
		return false;
	else 
		return length() < rhs.length();
}


unsigned char Rule::maxRuleLength() const
{
	return m_validator.maxRuleLength();
}

const HostSelectors &Rule::selectors() const
{
    return m_selectors;
}

/*
DeviceSelectors &Rule::deviceSelectors()
{
    return m_device_selectors;
}
*/
std::ostream& operator<<(std::ostream& os, RulePtr r)
{
	std::stringstream selectors;
	if (!r->selectors().empty())
	{
        /*
        const Selector& last_sel = r->selectors().back();
        for (const Selector& s : r->selectors())
		{
            selectors << s.toString();
			if (last_sel != s)
				selectors << " AND ";
		}
        */
        const HostSelectors& sel = r->selectors();
        const unsigned int sel_size = sel.size();
        for (unsigned int i = 0; i < sel_size; ++i)
        {
            selectors << sel[i].toString();
            if (i != sel_size - 1)
                selectors << " AND ";
        }
	}
	
	os << "IF " << selectors.str() << " THEN CLASS = " << (int)r->targetClass() << " " << r->distributionToString() <<" quality: " << r->quality() << "\n";
	return os;
}

std::string Rule::distributionToString() const
{
	std::string str = "[";
	for (const auto& d : m_rule_dist)
	{
		str.append(std::to_string(d) + " ");
	}
	str.back() = ']'; // delete last space
	return str;

}

float Rule::quality() const
{
	return m_quality;
}
