cmake_minimum_required(VERSION 3.16)

set(this_tests simple_xml_parser_tests)
project(${this_tests})

set(header_files ../include/simple_xml.h)
set(source_files tests.cpp)

if(BUILD_TESTING)
  add_executable(${this_tests} ${header_files} ${source_files})
  target_include_directories(${this_tests} PUBLIC ../include)
  target_link_libraries(${this_tests} PRIVATE simple_xml_parser)
  target_link_libraries(${this_tests} PRIVATE GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(${this_tests})
endif()
