#include <iostream>
#include "DataFileReader.cuh"
#include "CN2UnorderedLearner.h"
//#include "RuleClassifier.h"
#include "Measure.h"
#include "DataContainer.cuh"

//CN2UnorderedLearner learner;

void train()
{
    //learner.fit(DataFileReader::getInstance().trainData());
    //learner.printRules();
}
void test()
{
    //RuleClassifier rc(DataFileReader::getInstance().testData(), learner.rules());
    //rc.run();
}
int main(int argc, char* argv[])
{
    const char* filename = argv[1];
    if ( filename )
    {
        try
        {
            DataFileReader::run(filename, 3000000);
            DataFileReader& fr = DataFileReader::getInstance();
            unsigned int alive_flag_position = fr.attributes().size() + 2; // attributes + class + flag
            DataContainer data(fr.trainData(), fr.deviceData(), fr.distribution().size(), alive_flag_position);
            CN2UnorderedLearner learner;
            learner.fit(data);
            learner.printRules();

            //std::cout << measure<>::execution(train) <<"ms" << "\n";
            //std::cout << measure<>::execution(test) << "ms" << "\n";
            /*
            thrust::host_vector<float>& h_vec = DataFileReader::getInstance().trainData();
            size_t line = DataFileReader::getInstance().attributes().size() + 2;
            for (size_t i = 0; i < h_vec.size(); ++i)
            {
                if (i != 0 && (i % line) == 0 )
                    std::cout << "\n";
                std::cout  << h_vec[i] << " ";
            }

            std::cout << "\n";
            */
            DataFileReader::freeDeviceData();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    else
    {
        std::cout << "input file is not set" << std::endl;
    }

	
	return 0;
}
