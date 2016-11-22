project(CN2_Rules)
cmake_minimum_required(VERSION 2.8)

find_package(CUDA QUIET REQUIRED)

# Pass options to NVCC
set( CUDA_PROPAGATE_HOST_FLAGS OFF )
set( CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3 -std=c++11 --expt-extended-lambda)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

cuda_add_executable(
	    CN2_Rules
		Attribute.h
		CN2UnorderedLearner.cpp
		CN2UnorderedLearner.h
		DataFileReader.cpp
		DataFileReader.h
		Rule.cpp
		Rule.h
		RuleHunter.cpp
		RuleHunter.h
		Selectors.cpp
		Selectors.h
		Validators.cpp
		Validators.h
		Measure.h
		RuleClassifier.cpp
		RuleClassifier.h
		main.cpp
)

