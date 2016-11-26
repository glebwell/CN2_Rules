#include "Validators.h"
#include "Rule.h"

GuardianValidator::GuardianValidator(unsigned char length, unsigned char min_cov_examples): 
	m_max_rule_length(length), m_min_covered_examples(min_cov_examples)
{

}

bool GuardianValidator::operator()(const Rule& r) const
{
    return r.coveredExamplesCount() >= m_min_covered_examples
		&& r.length() <= m_max_rule_length
		&& compareDistributions(r)
		&& r.distribution()[r.targetClass()] != 0; // must find examples of rule class
}

bool GuardianValidator::compareDistributions(const Rule& r) const
{
	const auto& parent_rule = r.parent();
	if (parent_rule)
		return r.distribution() != parent_rule->distribution(); // distibution of parent rule must be not equal
	else
		return true;
}

unsigned char GuardianValidator::maxRuleLength() const
{
	return m_max_rule_length;
}

unsigned char GuardianValidator::minCoveredExamples() const
{
	return m_min_covered_examples;
}
