cmake_minimum_required(VERSION 3.16)

set(this simple_xml_parser)
project(${this} VERSION 0.0.1 
	DESCRIPTION "Simple XML parsing and writing C++ library" 
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_POSITION_INDEPEDENT_CODE ON)

set(header_files include/simple_xml.h include/stl_helper_functions.hpp include/detail/stl_helper_functions_impl.hpp)
set(source_files src/simple_xml.cpp)

add_library(${this} STATIC ${header_files} ${source_files})

target_include_directories(${this} PRIVATE include)

include(CTest)

add_subdirectory(googletest)

add_subdirectory(tests)
