#pragma once
#include "DataFileReader.cuh"
#include "RuleHunter.h"

class DataContainer;

class CN2UnorderedLearner
{
	std::vector<RulePtr> m_rules;
	RuleHunter m_hunter;
public:
    void fit(DataContainer& data);
	void printRules() const;
	const std::vector<RulePtr>& rules() const;
private:
    bool positiveRemainingDataStopping(const DataContainer& data, unsigned char target_class) const;
    void coverAndRemove(DataContainer& data, RulePtr r) const;
	bool ruleStopping(RulePtr r) const;
    RulePtr generateDefaultRule() const;
    void findRules(DataContainer& data, unsigned char target_class);
	void filterRulesByQuality();
};
