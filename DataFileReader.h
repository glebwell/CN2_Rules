#pragma once
#include <vector>
#include <map>
#include <memory>
#include <list>
#include "Attribute.h"


using Examples = std::list<std::vector<float>>;
// vector of examples, index of vector identify class
using DataVector = std::vector<Examples>;
// distribution of examples in class 
using Distribution = std::vector<size_t>;

class DataFileReader
{
public:
	const std::vector<Attribute>& attributes() const;
	DataVector& trainData();
	DataVector& testData();
	const Distribution& distribution() const;
	static DataFileReader& getInstance();
	static void run(const char* filename, size_t train_data_size);
	float majorityQuality() const;

private:
	DataFileReader(const char* filename, size_t train_data_size);
	void parseHeader(const std::string& line);
	void parseValueLine(const std::string& line, bool put_to_train);
	void insertToVector(DataVector& vec, int class_val, std::vector<float>&& data);
	void calcClassDistribution();
	void calcMajorityQuality();
	std::vector<std::string> tokenize(const std::string& str, char delim = ' ', bool trimEmpty = true) const;
	
	std::vector<Attribute> m_attributes;
	Distribution m_class_dist;
	DataVector m_train_data;
	DataVector m_test_data;
	static std::unique_ptr<DataFileReader> m_self;
	float m_majority_quality;
	bool m_header_parsed;
};

class DataFileReaderException : public std::exception
{
	std::string m_msg;
public:
	DataFileReaderException(std::string&& msg);
    virtual const char *what() const noexcept override;
};
