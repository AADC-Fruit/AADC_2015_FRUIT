
cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)

cmake_policy(SET CMP0002 NEW)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0011 NEW)

project(AADC_base_filter_tests)

set(ADTF_TEST_RESULTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/results")
 
find_package(ADTF REQUIRED PATHS "C:/Program Files/ADTF/2.12.0")
#find_package(AADC REQUIRED)

#set the install prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "The install directory must point for tests to the AADC root dir, where you can find the bin dir." FORCE) 
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

enable_testing()
 
add_subdirectory(module)
add_subdirectory(integration)
#add_subdirectory(libraries)
