CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

project(TAPIR_PACKAGE CXX C)

INCLUDE( ManageString.cmake )
INCLUDE( configuration.cmake.in )
MESSAGE( STATUS )

INCLUDE( registry/CMakeLists.txt )

# find optional packages
MESSAGE( STATUS )
MESSAGE( STATUS "Searching for additional packages ---------------------------------------------" )
FIND_PACKAGE(Doxygen)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Doxygen DEFAULT_MSG DOXYGEN_EXECUTABLE)

MESSAGE (STATUS ${CMAKE_OSX_ARCHITECTURES})

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
SET(CMAKE_INSTALL_PREFIX
"${TAPIR_PACKAGE_SOURCE_DIR}" CACHE PATH "TAPIR install prefix" FORCE
)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

MESSAGE( STATUS )
MESSAGE( STATUS "--Tapir package options -------------------------------------------------------" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX   = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE       = ${CMAKE_BUILD_TYPE}")
MESSAGE( STATUS "Change a value with: cmake -D<VAR>=<VALUE>" )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS )

add_subdirectory(src)

# doc target
IF(DOXYGEN_FOUND)
  ADD_CUSTOM_TARGET(doc COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE} 
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF()
