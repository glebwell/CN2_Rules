#include <iostream>
#include <algorithm>
#include "CN2UnorderedLearner.h"
#include "DataContainer.cuh"
#include "Rule.h"

void CN2UnorderedLearner::fit(DataContainer& data)
{
    DataFileReader& reader = DataFileReader::getInstance();
    GuardianValidator::setMajorityQuality(reader.majorityQuality());
    size_t classAmount = reader.distribution().size();
    for (unsigned char curr_class = 0; curr_class < classAmount; ++curr_class)
    {
        findRules(data, curr_class);
    }
    //filterRulesByQuality();
    // sort by class
    std::sort(m_rules.begin(), m_rules.end(), [](const RulePtr& r1, const RulePtr& r2) {return r1->targetClass() < r2->targetClass(); });
    m_rules.push_back(generateDefaultRule());
}

void CN2UnorderedLearner::findRules(DataContainer& data, unsigned char tc)
{
    while (!positiveRemainingDataStopping(data, tc)) // TODO: positive remaining data stopping
	{
		RulePtr new_rule = m_hunter(data, tc, m_rules);
        if (!new_rule /*|| ruleStopping(new_rule)*/)
			break;
        coverAndRemove(data, new_rule); // TODO: coverAndRemove
		m_rules.push_back(new_rule);
	}
}

bool CN2UnorderedLearner::positiveRemainingDataStopping(const DataContainer& data, unsigned char target_class) const
{
    /*
	const Examples& ex = data[target_class];
	size_t num_possible = ex.size();
	size_t min_covered_examples = 1; // TODO: fix it
	return num_possible < min_covered_examples;
    */
    return false;
}
void CN2UnorderedLearner::coverAndRemove(DataContainer &data, RulePtr r) const
{
    data.removeKernelCall(r->selectors());
}
bool CN2UnorderedLearner::ruleStopping(RulePtr r) const
{
	// LRS significance rule stopping
	return r->isSignificant(true);
}

RulePtr CN2UnorderedLearner::generateDefaultRule() const
{
	RulePtr default_rule = std::make_shared<Rule>();
    default_rule->filterAndStore( 255 );
	default_rule->doEvaluate();
	//default_rule->createModel();
	return default_rule;
}

void CN2UnorderedLearner::printRules() const
{
	for (const auto& r : m_rules)
		std::cout << r;
}

void CN2UnorderedLearner::filterRulesByQuality()
{
    float majority_quality = DataFileReader::getInstance().majorityQuality();
    std::vector<RulePtr> filtered_rules;
    filtered_rules.reserve(m_rules.size());
    std::copy_if(m_rules.cbegin(), m_rules.cend(), std::back_inserter(filtered_rules),
                 [majority_quality](RulePtr r) { return r->quality() > majority_quality;}
    );
    m_rules = std::move(filtered_rules);
}


const std::vector<RulePtr>& CN2UnorderedLearner::rules() const
{
	return m_rules;
}
