#include <iostream>
#include <algorithm>
#include "RuleClassifier.h"

RuleClassifier::RuleClassifier(const DataVector& test_data, const std::vector<RulePtr>& rules): 
m_total_examples_count(0), m_success_prediction_count(0), m_test_data(test_data), m_rules(rules)
{

}

void RuleClassifier::run()
{
	m_failes_by_class.resize(m_test_data.size());
	for (unsigned char class_id = 0; class_id < m_test_data.size(); ++class_id)
	{
		const Examples& exs = m_test_data[class_id];
		for (const auto& ex : exs)
		{
			++m_total_examples_count;
			unsigned char predicted_class = predict(ex);
			if (predicted_class == class_id)
				++m_success_prediction_count;
			else
				++m_failes_by_class[class_id];
		}
	}

	std::cout << "[" << m_success_prediction_count << " " << m_total_examples_count << "]" << " "
		<< (float) m_success_prediction_count/m_total_examples_count * 100 << "%" << "\n";

	std::string str = "[";
	for (const auto& f : m_failes_by_class)
	{
		str.append(std::to_string(f) + " ");
	}
	str.back() = ']'; // delete last space

	std::cout << "Failes:" << str << "\n";
}

unsigned char RuleClassifier::predict(const std::vector<float>& example) const
{
	Distribution dist(m_test_data.size()); // size of dist is a classes count
	std::vector<RulePtr> firedRules;
	auto end = --m_rules.cend(); // go for all rules, except last - default rule
	//
	// copy fired rules
	// 
	std::copy_if(m_rules.cbegin(), end, std::back_inserter(firedRules),
		[&](const RulePtr& r) {return r->testRule(example); });


	if (firedRules.empty()) // apply default
	{
		RulePtr default_rule = m_rules.back(); // default rule is last
		auto begin = default_rule->distribution().cbegin();
		auto end = default_rule->distribution().cend();

		// TODO: check if max is not one
		// return position of max element - it's a class_id
		return std::distance(begin, std::max_element(begin, end));
	}
	else // apply fired rules
	{
		//
		// Check that fired rules target class is the same
		// 

		bool is_same = std::adjacent_find(firedRules.cbegin(), firedRules.cend(),
			[](const RulePtr& r1, const RulePtr& r2) {return r1->targetClass() != r2->targetClass(); }) == firedRules.cend();

		if (is_same) // all classes are the same, just return class_id
		{
			return firedRules.back()->targetClass();
		}
		else
		{
			for (const auto& r : firedRules)
			{
				const Distribution& rule_dist = r->distribution();
				// accumulate sum of fired rules distribution
				std::transform(rule_dist.cbegin(), rule_dist.cend(), dist.cbegin(), dist.begin(),
					[](size_t s, size_t d) {return s + d; });
			}

			// return position of max element - it's a class_id
			// TODO: check if max is not one
			return std::distance(dist.cbegin(), std::max_element(dist.cbegin(), dist.cend()));
		}
	}
}