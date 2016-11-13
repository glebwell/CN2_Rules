#include <iostream>
#include "DataFileReader.h"
#include "CN2UnorderedLearner.h"
#include "RuleClassifier.h"
#include "Measure.h"

CN2UnorderedLearner learner;

void train()
{
	learner.fit(DataFileReader::getInstance().trainData());
	learner.printRules();
}
void test()
{
    //RuleClassifier rc(DataFileReader::getInstance().testData(), learner.rules());
    //rc.run();
}
int main(int argc, char* argv[])
{
    const char* filename = argv[1];
	try
	{
        DataFileReader::run(filename, 3000000);
		std::cout << measure<>::execution(train) <<"ms" << "\n";
		std::cout << measure<>::execution(test) << "ms" << "\n";
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}

	
	return 0;
}
