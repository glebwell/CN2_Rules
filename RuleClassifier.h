#pragma once
#include "DataFileReader.cuh"
#include "Rule.h"

/*
class RuleClassifier
{
	size_t m_total_examples_count;
	size_t m_success_prediction_count;
	const DataVector& m_test_data;
	const std::vector<RulePtr>& m_rules;
	std::vector<size_t> m_failes_by_class;

public:
	RuleClassifier(const DataVector& test_data, const std::vector<RulePtr>& rules);
	void run();
private:
	// return predicted class 
	unsigned char predict(const std::vector<float>& example) const;

};
*/

