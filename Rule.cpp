#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include "Rule.h"

GuardianValidator Rule::m_validator;

Rule::Rule(RulePtr r) : m_parent_rule(r), m_covering(0), m_quality(0), m_target_class(-1)
{
	if (m_parent_rule) // copy selectors from parent rule
	{
		m_selectors = r->selectors();
	}
}


bool Rule::applySelectors(const std::vector<float>& example) const
{ 
    for (const auto& sel : m_selectors)
	{
        if ((*sel)(example[sel->m_attr_index]) == false) // call selector
			return false;
	}
	return true;
}

void Rule::filterAndStore(const DataVector& const_data, unsigned char target_class)
{
    m_target_class = target_class; // init class value
    if (m_selectors.empty()) // don't calc dist for default rule
    {
        m_rule_dist = DataFileReader::getInstance().distribution();
        m_covering = std::accumulate(m_rule_dist.cbegin(), m_rule_dist.cend(), 0u);
    }
    else // calc covered examples and distribution
    {
        DataVector& data = const_cast<DataVector&>(const_data); // make possible to store non const iterators in m_covered_examples
        size_t data_size = data.size();
        if (m_rule_dist.size() < data_size)
            m_rule_dist.resize(data_size);

        size_t covery_in_class;
        for (unsigned char c = 0; c < data_size; ++c)
        {
            covery_in_class = 0;
            Examples& ex = data[c];
            for (Examples::iterator i = ex.begin(); i != ex.end(); ++i)
            {
                if (applySelectors(*i))
                {
                    ++covery_in_class;
                    m_covered_examples.push_back( {c, i} );
                }
            }
            m_covering += covery_in_class;
            m_rule_dist[c] = covery_in_class;
        }
    }
}

bool Rule::testRule(const std::vector<float>& example) const
{
	return applySelectors(example);
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

bool Rule::isSignificant(bool useInitialClassDist /*= false*/) const
{
	const Distribution* p_dist;
	if (useInitialClassDist)
		p_dist = &DataFileReader::getInstance().distribution();
	else if (m_parent_rule)
		p_dist = &m_parent_rule->m_rule_dist;
	else
		return true;

	unsigned char tc = m_target_class;
	float tc_class_dist = (float)m_rule_dist[tc];
	float tc_p_class_dist = (float)((*p_dist)[tc]);
    float dist_sum = std::accumulate(m_rule_dist.cbegin(), m_rule_dist.cend(), 0.f);
	float p_dist_sum = std::accumulate(p_dist->cbegin(), p_dist->cend(), 0.f);
	std::vector<float> x, y;
	if (tc != -1)
	{
		x = { tc_class_dist, dist_sum - tc_class_dist };
		y = { tc_p_class_dist, p_dist_sum - tc_p_class_dist };
	}
	else
	{
		const Distribution& p_dist_ref = *p_dist;
		// suppress warnings
		for (size_t i = 0; i < m_rule_dist.size(); ++i)
		{
			x.push_back((float)m_rule_dist[i]);
			y.push_back((float)p_dist_ref[i]);
		}
		
		//x.assign(m_rule_dist.cbegin(), m_rule_dist.cend());
		//y.assign(p_dist->cbegin(), p_dist->cend());
		//std::copy(m_rule_dist.cbegin(), m_rule_dist.cend(), std::back_inserter(x));
		//std::copy(p_dist->cbegin(), p_dist->cend(), std::back_inserter(y));
	}

	//
	// calculate likelihood ratio statistic
	//

	if (x[0] == 0)
		x[0] = (float)1e-5;
	if (y[0] == 0)
		y[0] = (float)1e-5;

	float x_sum = std::accumulate(x.cbegin(), x.cend(), 0.f);
	float y_sum = std::accumulate(y.cbegin(), y.cend(), 0.f);
	float div = x_sum / y_sum;
	std::for_each(y.begin(), y.end(), [div](float v){ return v * div;});

	float sum = 0;
	for (size_t i = 0; i < x.size(); i++)
		sum += x[i] * std::log(x[i] / y[i]);

	float lrs = 2 * sum;
	return lrs > 0;
}

void Rule::addSelector(const Selector* s)
{
	m_selectors.push_back(s);
}

unsigned char Rule::targetClass() const
{
	return m_target_class;
}

size_t Rule::coveredExamplesCount() const
{
	return m_covering;
}

const CoveryMap& Rule::coveryMap() const
{
	return m_covered_examples;
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
			if (*m_selectors[i] != *other.m_selectors[i])
				return false;
		}
	}

	return true;
}

bool Rule::operator!=(const Rule& rhs) const
{
	return m_quality != rhs.m_quality 
		&& m_covering != rhs.m_covering && m_covered_examples != rhs.m_covered_examples;
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

const std::vector<const Selector *> &Rule::selectors() const
{
	return m_selectors;
}

std::ostream& operator<<(std::ostream& os, RulePtr r)
{
	std::stringstream selectors;
	if (!r->selectors().empty())
	{
		const auto& last_sel = r->selectors().back();
		for (const auto& s : r->selectors())
		{
			selectors << s->toString();
			if (last_sel != s)
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
