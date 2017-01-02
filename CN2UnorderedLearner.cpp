#include <iostream>
#include <algorithm>
#include "CN2UnorderedLearner.h"
#include "Rule.h"

CN2UnorderedLearner::CN2UnorderedLearner(float min_rule_quality, unsigned char beam_width):
   m_min_rule_quality(min_rule_quality), m_hunter(beam_width), m_avg_quality(0), m_max_quality(0), m_db_coverage(0)
{

}

void CN2UnorderedLearner::fit(DataVector& data)
{
    size_t classAmount = DataFileReader::getInstance().distribution().size();
    for (unsigned char curr_class = 0; curr_class < classAmount; ++curr_class)
    {
        findRules(data, curr_class);
    }
    filterRulesByQuality();
    // sort by class
    std::sort(m_rules.begin(), m_rules.end(), [](const RulePtr& r1, const RulePtr& r2) {return r1->targetClass() < r2->targetClass(); });
    m_rules.push_back(generateDefaultRule(data));
}

void CN2UnorderedLearner::findRules(DataVector& data, unsigned char tc)
{
	while (!positiveRemainingDataStopping(data, tc))
	{
		RulePtr new_rule = m_hunter(data, tc, m_rules);
		if (!new_rule || ruleStopping(new_rule))
			break;
		coverAndRemove(data, new_rule);
		m_rules.push_back(new_rule);
	}
}

bool CN2UnorderedLearner::positiveRemainingDataStopping(const DataVector& data, unsigned char target_class) const
{
	const Examples& ex = data[target_class];
	size_t num_possible = ex.size();
	size_t min_covered_examples = 1; // TODO: fix it
	return num_possible < min_covered_examples;
}
void CN2UnorderedLearner::coverAndRemove(DataVector& data, RulePtr r) const
{
    const CoveryMap& covery = r->coveryMap();
    for (const auto& class_iter_pair : covery)
        data[class_iter_pair.first].erase(class_iter_pair.second);
}
bool CN2UnorderedLearner::ruleStopping(RulePtr r) const
{
	// LRS significance rule stopping
	return r->isSignificant(true);
}

RulePtr CN2UnorderedLearner::generateDefaultRule(const DataVector& data) const
{
	RulePtr default_rule = std::make_shared<Rule>();
	default_rule->filterAndStore(data, -1);
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
    std::vector<RulePtr> filtered_rules;
    filtered_rules.reserve(m_rules.size());
    std::copy_if(m_rules.cbegin(), m_rules.cend(), std::back_inserter(filtered_rules),
                 [this](RulePtr r) { return r->quality() > m_min_rule_quality;}
    );
    m_rules = std::move(filtered_rules);
}


const std::vector<RulePtr>& CN2UnorderedLearner::rules() const
{
    return m_rules;
}

void CN2UnorderedLearner::calcRuleInfo()
{
    RulePtr default_rule = m_rules.back();
    const Distribution& default_rule_dist = default_rule->distribution();
    const unsigned int classes_count = default_rule_dist.size();
    size_t total_objects = 0;
    total_objects = std::accumulate(default_rule_dist.cbegin(), default_rule_dist.cend(), total_objects);
    Distribution db_coverage_dist( default_rule_dist.size() );

    auto prev_end = --m_rules.end();
    float quality;
    for (auto it = m_rules.cbegin(); it != prev_end; ++it)
    {
        quality = (*it)->quality();
        if (m_max_quality < quality)
            m_max_quality = quality;

        for (unsigned int i = 0; i < classes_count; ++i)
        {
            db_coverage_dist[i] += (*it)->distribution()[i];
        }
        m_avg_quality += quality;
    }

    size_t covered_objects = 0;
    covered_objects = std::accumulate(db_coverage_dist.cbegin(), db_coverage_dist.cend(), covered_objects);
    m_db_coverage = covered_objects / (float)total_objects;
    m_avg_quality /= rulesCount();
}

size_t CN2UnorderedLearner::rulesCount() const
{
    // exclude default rule
    return m_rules.size() - 1;
}

float CN2UnorderedLearner::averageQuality() const
{
    return m_avg_quality;
}

float CN2UnorderedLearner::maxQuality() const
{
    return m_max_quality;
}

float CN2UnorderedLearner::databaseCoverage() const
{
    return m_db_coverage;
}

