#pragma once
#include "Rule.h"
#include "DataFileReader.h"
#include "Selectors.h"

class RuleHunter
{
	const unsigned char m_beam_width;
public:
	RuleHunter(unsigned char beam_width = 10);
	// find best rule
	RulePtr operator()(const DataVector& data, unsigned char target_class, const std::vector<RulePtr>& existing_rules) const;
private:
	RulePtr initializeRule(const DataVector& data, 
		unsigned char target_class) const;
	
	std::vector<RulePtr> refineRule(const DataVector& data, RulePtr candidate_rule) const;
	bool isExist(RulePtr tested_rule, const std::vector<RulePtr>& existing_rules) const;
	std::vector<RulePtr> cutRules(const std::vector<RulePtr>& rules_to_cut) const;
	std::vector<SelectorPtr> findNewSelectors(const DataVector& data, RulePtr r) const;
	void generateSelectors(Attribute::attr_type type, float value, unsigned char attr_idx, std::vector<SelectorPtr>& vec_to_store) const;
	std::vector<float> makeSamples(const DataVector& data, unsigned char attr_idx) const;
};