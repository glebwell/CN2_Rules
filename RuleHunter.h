#pragma once
#include "Rule.h"
#include "DataFileReader.h"
#include "Selectors.h"
#include "SelectorGenerator.h"

class RuleHunter
{
	const unsigned char m_beam_width;
public:
    RuleHunter(unsigned char beam_width = 1);
	// find best rule
    RulePtr operator()(const DataVector& data, unsigned char target_class, const std::vector<RulePtr>& existing_rules);
private:
	RulePtr initializeRule(const DataVector& data, 
		unsigned char target_class) const;
	
    std::vector<RulePtr> refineRule(const DataVector& data, RulePtr candidate_rule);
	bool isExist(RulePtr tested_rule, const std::vector<RulePtr>& existing_rules) const;
	std::vector<RulePtr> cutRules(const std::vector<RulePtr>& rules_to_cut) const;
    std::vector<const Selector *> findNewSelectors(const DataVector& data, RulePtr r);
	std::vector<float> makeSamples(const DataVector& data, unsigned char attr_idx) const;

    SelectorGenerator m_gen;
};
