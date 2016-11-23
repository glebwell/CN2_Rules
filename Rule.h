#pragma once
#include <iosfwd>
#include "Selectors.h"
#include "DataFileReader.h"
#include "Validators.h"

class Rule;
using ExIterator = Examples::iterator;
// covered examples positions mapped to attribite index
using CoveryMap = std::vector<std::pair<unsigned char, ExIterator>>;
using RulePtr = std::shared_ptr<Rule>;

class Rule
{
    std::vector<const Selector*> m_selectors;
	CoveryMap m_covered_examples;
	Distribution m_rule_dist;
	std::shared_ptr<Rule> m_parent_rule;
	size_t m_covering;
	float m_quality;
	unsigned char m_target_class;
	static GuardianValidator m_validator;

public:
	Rule(RulePtr r = nullptr);
	bool operator==(const Rule& rhs) const;
	bool operator!=(const Rule& rhs) const;
	bool operator<(const Rule& rhs) const;
	bool operator>(const Rule& rhs) const;

    void addSelector(const Selector *s);
	// Apply data and target class to a rule
    void filterAndStore(const DataVector &const_data, unsigned char target_class);
	// Return True if the rule passes the general validator's requirements
	bool isValid() const;
	// Return True if the rule passes the significance validator's requirements(is significant in regard to its parent).
	bool isSignificant(bool useInitialClassDist = false) const;
	// Evaluate the rule's quality and complexity.
	void doEvaluate();
	// test rule on test data
	bool testRule(const std::vector<float>& example) const;

	unsigned char targetClass() const;
	size_t coveredExamplesCount() const;
	size_t length() const;
	const Distribution& distribution() const;
	std::shared_ptr<Rule> parent() const;
	unsigned char maxRuleLength() const;
	const CoveryMap& coveryMap() const;
    const std::vector<const Selector*>& selectors() const;
	std::string distributionToString() const;
	float quality() const;
private:
	bool applySelectors(const std::vector<float>& example) const;
	bool compareSelectors(const Rule& other) const;
};

std::ostream& operator<<(std::ostream& os, RulePtr r);
