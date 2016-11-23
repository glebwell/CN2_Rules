#include <algorithm>
#include <set>
#include "RuleHunter.h"

RuleHunter::RuleHunter(unsigned char beam_width) : m_beam_width(beam_width)
{

}

RulePtr RuleHunter::initializeRule(const DataVector& data, unsigned char target_class) const
{
	RulePtr default_rule = std::make_shared<Rule>();
	default_rule->filterAndStore(data, target_class);
	return default_rule->isValid() ? default_rule : nullptr;
}

RulePtr RuleHunter::operator()(const DataVector& data, unsigned char target_class, const std::vector<RulePtr>& existing_rules)
{
	RulePtr best_rule = initializeRule(data, target_class);
	if (!best_rule)
		return nullptr;
	else
	{
		std::vector<RulePtr> rules{ best_rule };
		
		while (!rules.empty())
		{
			std::vector<RulePtr> candidates = std::move(rules);
			for (const auto& cand : candidates)
			{
				// try to refine the rules
				std::vector<RulePtr> new_rules = refineRule(data, cand);
				if (!new_rules.empty())
				{
					if (rules.capacity() < new_rules.size())
						rules.reserve(new_rules.size());

					std::copy(new_rules.begin(), new_rules.end(), std::back_inserter(rules));
					for (const auto& rule : new_rules)
					{
						if (*rule > *best_rule && /*rule->isSignificant() && */!isExist(rule, existing_rules))
							best_rule = rule;
					}
					static const auto& comp = [](const RulePtr& r1, const RulePtr& r2) { return *r1 > *r2; };
					std::sort(rules.begin(), rules.end(), comp);
					// cut rules accoding to beam width
					rules = cutRules(rules);
				}	
			}
		}

		//if (existing_rules.cend() != std::find(existing_rules.cbegin(), existing_rules.cend(), best_rule)) // already exist
			//return nullptr;
		auto iter = std::find_if(existing_rules.cbegin(), existing_rules.cend(), [best_rule](const RulePtr& r) {return *r == *best_rule; });
		if (iter != existing_rules.cend())
			return nullptr;

	}
	return best_rule;
}

bool RuleHunter::isExist(RulePtr tested_rule, const std::vector<RulePtr>& ex_rules) const
{
	return ex_rules.cend() != std::find(ex_rules.cbegin(), ex_rules.cend(), tested_rule);
}

std::vector<RulePtr> RuleHunter::refineRule(const DataVector& data, RulePtr candidate_rule)
{
	std::vector<RulePtr> result;
	if (candidate_rule && candidate_rule->length() <= candidate_rule->maxRuleLength())
	{
        std::vector<const Selector*> possible_selectors = findNewSelectors(data, candidate_rule);
		for (const auto& sel : possible_selectors)
		{
			RulePtr new_rule = std::make_shared<Rule>(candidate_rule);
			new_rule->addSelector(sel);
			new_rule->filterAndStore(data, candidate_rule->targetClass());
			if (new_rule->isValid())
			{
				new_rule->doEvaluate();
				result.push_back(std::move(new_rule));
			}	
		}
	}
	return result;
}

std::vector<RulePtr> RuleHunter::cutRules(const std::vector<RulePtr>& rules_to_cut) const
{
	if (rules_to_cut.size() > m_beam_width)
		return std::vector<RulePtr>(rules_to_cut.cbegin(), rules_to_cut.cbegin() + m_beam_width);
	else
		return rules_to_cut;
}

std::vector<const Selector*> RuleHunter::findNewSelectors(const DataVector& data, RulePtr r)
{
    std::vector<const Selector*> possible_selectors;
	
	// To generate nw selectors, we need unique values 
	// Index of vector identify index of example attribute. That vector containes unique values of attributes.
	const std::vector<Attribute>& attr_info = DataFileReader::getInstance().attributes();
	size_t attr_size = attr_info.size();
	std::vector<std::set<float>> attr_val_set(attr_size);
	float value;
    Attribute::attr_type attr_type;
	const CoveryMap& map = r->coveryMap();

	if (map.empty()) // generate for all data
	{
		for (const Examples& ex_list : data) // for data vector(classes)
		{
			for (const std::vector<float>& ex : ex_list) // for all examples
			{
				for (unsigned char attr_idx = 0; attr_idx < attr_size; ++attr_idx) // for all attributes indexes
				{
					auto& set = attr_val_set[attr_idx];
					value = ex[attr_idx];
					if (set.cend() == set.find(value)) // if this value is not in set - skipping selectors generation
					{
						set.insert(value);
						// generate selectors
                        attr_type = attr_info[attr_idx].m_type;
                        m_gen.store(attr_type, value, attr_idx, possible_selectors);
					}		
				}
			}
		}
	}
	else // generate for covered by rule data 
	{	
		for (const auto& class_iter_pair : map) // for all iterators
		{
			const std::vector<float>& example = *class_iter_pair.second;
		    for (unsigned char attr_idx = 0; attr_idx < attr_size; ++attr_idx) // for all attributes indexes
		    {
				auto& set = attr_val_set[attr_idx];
				value = example[attr_idx];

				if (set.cend() == set.find(value)) // if this value is not in set - skipping selectors generation
				{
					set.insert(value);
					// generate selectors
                    attr_type = attr_info[attr_idx].m_type;
                    m_gen.store(attr_type, value, attr_idx, possible_selectors);
				}
			}
		}
	}
	return possible_selectors;
}


std::vector<float> RuleHunter::makeSamples(const DataVector& data, unsigned char attr_idx) const
{
	std::vector<float> samples;
	const std::vector<Examples>& exs_list = data;
	float min = exs_list[0].front()[attr_idx], max = exs_list[0].front()[attr_idx];
	float el;

	// find min and max
	for (const auto& exs : exs_list)
	{
		for (const auto& vec : exs)
		{
			el = vec[attr_idx];
			if (el > max)
				max = el;
			else if (el < min)
				min = el;
		}
	}
	
	float delta = (max - min) / 100;
	samples.reserve(100);
	std::generate(samples.begin(), samples.end(), [&min, &delta]() { float v = min; min += delta; return v; });
	return samples;
}
