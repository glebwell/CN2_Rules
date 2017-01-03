#pragma once
#include "DataFileReader.cuh"
#include "RuleHunter.h"

class DataContainer;

class CN2UnorderedLearner
{
	std::vector<RulePtr> m_rules;
	RuleHunter m_hunter;
    float m_min_rule_quality;
    float m_avg_quality;
    float m_max_quality;
    float m_db_coverage;

public:
    CN2UnorderedLearner(float min_rule_quality, unsigned char beam_width);
    void fit(DataContainer& data);
	void printRules() const;
	const std::vector<RulePtr>& rules() const;
    size_t rulesCount() const;
    float averageQuality() const;
    float maxQuality() const;
    float databaseCoverage() const;
private:
    bool positiveRemainingDataStopping(const DataContainer& data, unsigned char target_class) const;
    void coverAndRemove(DataContainer& data, RulePtr r) const;
    RulePtr generateDefaultRule() const;
    void findRules(DataContainer& data, unsigned char target_class);
	void filterRulesByQuality();
    void calcRuleInfo();
};
