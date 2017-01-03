#include <iostream>
#include "DataFileReader.cuh"
#include "CN2UnorderedLearner.h"
#include "Measure.h"
#include "DataContainer.cuh"

int main(int argc, char* argv[])
{

    if ( argc != 5 )
    {
        std::cout << "usage: <program> <filename> <max_rule_length> <beam_width> <min_rule_conf>\n";
    }
    else
    {
        try
        {
            const char* filename = argv[1];
            unsigned int max_rule_length = std::stoul( argv[2] );
            unsigned char beam_width = std::stoul( argv[3] );
            float min_rule_conf = std::stof( argv[4] );

            DataFileReader::run(filename, -1);
            DataFileReader& fr = DataFileReader::getInstance();
            GuardianValidator::setMaxRuleLength(max_rule_length);
            unsigned int alive_flag_position = fr.attributes().size() + 1; // attributes + class + flag
            DataContainer data(fr.trainData(), fr.deviceData(), fr.distribution().size(), alive_flag_position);
            CN2UnorderedLearner learner(min_rule_conf, beam_width);
            learner.fit(data);
            learner.printRules();

            std::cout << "Total rules : " << learner.rulesCount() << "\n";
            std::cout << "Average quality : " << learner.averageQuality() << "\n";
            std::cout << "Max quality : " << learner.maxQuality() << "\n";
            std::cout << "Database coverage: " << learner.databaseCoverage() * 100 << "%\n";

            DataFileReader::freeDeviceData();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }

	return 0;
}
