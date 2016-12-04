#pragma once
class Rule;

class GuardianValidator
{
    static float m_majority_quality;
	unsigned char m_max_rule_length;
	unsigned char m_min_covered_examples;

public:
    GuardianValidator(unsigned char length = 3, unsigned char min_cov_examples = 1);
    static void setMajorityQuality(float quality);
	bool operator()(const Rule& r) const;
	unsigned char maxRuleLength() const;
	unsigned char minCoveredExamples() const;
    bool checkQuality(const Rule &r) const;
private:
	bool compareDistributions(const Rule& r) const;
};


