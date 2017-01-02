#pragma once
#include "DataFileReader.h"
#include "RuleHunter.h"

class CN2UnorderedLearner
{
	std::vector<RulePtr> m_rules;
	RuleHunter m_hunter;
    float m_min_rule_quality;
    float m_avg_quality;
    float m_max_quality;
    float m_db_coverage;
public:
    CN2UnorderedLearner(float min_rule_quality, unsigned char beam_width = 1);
	void fit(DataVector& data);
	void printRules() const;
	const std::vector<RulePtr>& rules() const;
    void calcRuleInfo();
    size_t rulesCount() const;
    float averageQuality() const;
    float maxQuality() const;
    float databaseCoverage() const;
private:
	bool positiveRemainingDataStopping(const DataVector& data, unsigned char target_class) const;
	void coverAndRemove(DataVector& data, RulePtr r) const;
	bool ruleStopping(RulePtr r) const;
	RulePtr generateDefaultRule(const DataVector& data) const;
	void findRules(DataVector& data, unsigned char target_class);
	void filterRulesByQuality();
};
