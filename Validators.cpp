#include "Validators.h"
#include "Rule.h"

float GuardianValidator::m_majority_quality;

GuardianValidator::GuardianValidator(unsigned char length, unsigned char min_cov_examples):
    m_max_rule_length(length), m_min_covered_examples(min_cov_examples)
{

}

void GuardianValidator::setMajorityQuality(float quality)
{
    m_majority_quality = quality;
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

bool GuardianValidator::checkQuality(const Rule& r) const
{
    return r.quality() > m_majority_quality;
}
