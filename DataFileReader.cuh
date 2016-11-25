#pragma once
#include <vector>
#include <memory>
#include <list>
#include <map>

#include "Defines.cuh"
#include "Attribute.h"


class DataFileReader
{
public:

	const std::vector<Attribute>& attributes() const;
    HostDataVector &trainData();
    DeviceDataVector& deviceData();
    //DataVector& testData();
	const Distribution& distribution() const;
	static DataFileReader& getInstance();
	static void run(const char* filename, size_t train_data_size);
	float majorityQuality() const;
    static void freeDeviceData();

private:
	DataFileReader(const char* filename, size_t train_data_size);
	void parseHeader(const std::string& line);
	void parseValueLine(const std::string& line, bool put_to_train);
    //void insertToVector(HostDataVector& vec, int class_val, std::vector<float>&& data);
	void calcClassDistribution();
	void calcMajorityQuality();
	std::vector<std::string> tokenize(const std::string& str, char delim = ' ', bool trimEmpty = true) const;
	
	std::vector<Attribute> m_attributes;
    Distribution m_class_dist;
    HostDataVector m_host_train_data;
    static DeviceDataVector* m_device_train_data;


    std::map<float, size_t> m_class_dist_map;
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

