#pragma once
#include "DataFileReader.h"
#include "RuleHunter.h"

class CN2UnorderedLearner
{
	std::vector<RulePtr> m_rules;
	RuleHunter m_hunter;
public:
	void fit(DataVector& data);
	void printRules() const;
	const std::vector<RulePtr>& rules() const;
private:
	bool positiveRemainingDataStopping(const DataVector& data, unsigned char target_class) const;
	void coverAndRemove(DataVector& data, RulePtr r) const;
	bool ruleStopping(RulePtr r) const;
	RulePtr generateDefaultRule(const DataVector& data) const;
	void findRules(DataVector& data, unsigned char target_class);
	void filterRulesByQuality();
};