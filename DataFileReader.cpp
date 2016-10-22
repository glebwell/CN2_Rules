#include <fstream>
#include <algorithm>
#include <numeric>
#include "DataFileReader.h"

std::unique_ptr<DataFileReader> DataFileReader::m_self;

DataFileReader::DataFileReader(const char* filename, size_t train_data_size) :m_header_parsed(false)
{
	std::ifstream file(filename);
	std::string line;
	size_t line_number = 0;
	bool put_to_train;
	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			if ( !m_header_parsed )
				parseHeader(line);
			else
			{
				put_to_train = line_number < train_data_size;
				parseValueLine(line, put_to_train);
				++line_number;
			}
		}
		
	}
	else
	{
		throw DataFileReaderException(std::string("Fail to open file ") + filename);
	}
	
	calcClassDistribution();
	calcMajorityQuality();
}

const std::vector<Attribute>& DataFileReader::attributes() const
{
	return m_attributes;
}

DataVector& DataFileReader::trainData()
{
	return m_train_data;
}

DataVector& DataFileReader::testData()
{
	return m_test_data;
}

void DataFileReader::parseHeader(const std::string& line)
{
	/*
	* Header structure:
	* attr1[attr_type] attr2[attr_type] ...  attrN[attr_type]
	* where attr_type may be 'c' - continuous, or 'd' - discrete
	*/
	
	if (!m_header_parsed)
	{
		std::vector<std::string> tokens = tokenize(line);

		const auto& func = [](const std::string& str)
		{
			size_t pos = str.find("[");
			if (pos != std::string::npos)
			{
				char type = str[pos + 1];
				Attribute a;
				a.m_type = (type == 'c') ?
					Attribute::attr_type::CONTINUOUS : Attribute::attr_type::DISCRETE;
				a.m_name = str.substr(0, pos);
				return a;
			}
			else
			{
				throw DataFileReaderException(std::string("Parsing header is failed"));
			}
		};

		std::transform(tokens.cbegin(), tokens.cend(), std::back_inserter(m_attributes), func);
		m_header_parsed = true;
	}
	
}

void DataFileReader::parseValueLine(const std::string& line, bool put_to_train)
{
	auto tokens = tokenize(line);
	if (tokens.size() == m_attributes.size() + 1) // tokens count must be equal attributes amount + 1 (class)
	{
		// TODO: map string to int
 		int class_value = std::stoi(tokens.back());  // last token is a class
	    tokens.pop_back();
		std::vector<float> data;
		data.reserve(tokens.size());
		std::transform(tokens.cbegin(), tokens.cend(), std::back_inserter(data),
			[](const std::string& str) {return std::stod(str); }
		);
		if (put_to_train)
			insertToVector(m_train_data, class_value, std::move(data));
		else
			insertToVector(m_test_data, class_value, std::move(data));
	}
	else
	{
		throw DataFileReaderException(std::string("Incorrect structure of data file, "
            "amount of values in line must be equal " + std::to_string(m_attributes.size() + 1)));
	}
}

void DataFileReader::insertToVector(DataVector& vec, int class_val, std::vector<float>&& data)
{
	if (vec.size() < (DataVector::size_type)class_val + 1)
		vec.resize(class_val+1);
	vec[class_val].push_back(std::move(data)); // put data in existing vector 
}

std::vector<std::string> DataFileReader::tokenize(const std::string& str, char delim /*= ' '*/, bool trimEmpty /*= true*/) const
{
	std::vector<std::string> tokens;
	std::string::size_type pos, lastPos = 0, length = str.length();

	while (lastPos < length + 1)
	{
		pos = str.find_first_of(delim, lastPos);
		if (pos == std::string::npos)
		{
			pos = length;
		}

		if (pos != lastPos || !trimEmpty)
			tokens.emplace_back(str.data() + lastPos,pos - lastPos);

		lastPos = pos + 1;
	}

	return tokens;
}

DataFileReader& DataFileReader::getInstance()
{
	if (m_self)
		return *m_self;
	else
		throw std::logic_error("DataFileReader is not initialized");
}

void DataFileReader::run(const char* filename, size_t train_data_size)
{
	//m_self = std::make_unique<DataFileReader>(filename, train_data_size);
	m_self.reset(new DataFileReader(filename, train_data_size));
}

void DataFileReader::calcClassDistribution()
{
	// calculate class <-> data count
	m_class_dist.reserve(m_test_data.size());
	for (const auto& examples_vec : m_train_data)
		m_class_dist.push_back(examples_vec.size());
}

const Distribution& DataFileReader::distribution() const
{
	return m_class_dist;
}

float DataFileReader::majorityQuality() const
{
	return m_majority_quality;
}

void DataFileReader::calcMajorityQuality()
{
	size_t k = m_class_dist.size();
	size_t max = *std::max_element(m_class_dist.cbegin(), m_class_dist.cend());
	size_t covering = std::accumulate(m_class_dist.cbegin(), m_class_dist.cend(), 0u);
	m_majority_quality = (max + 1) / (float)(covering + k);
}

DataFileReaderException::DataFileReaderException(std::string&& msg) : m_msg(std::move(msg))
{

}

const char * DataFileReaderException::what() const noexcept
{
	return m_msg.c_str();
}
