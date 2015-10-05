CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

project(CLSQUARE_PACKAGE CXX C)

INCLUDE( ManageString.cmake )
INCLUDE( configuration.cmake.in )
MESSAGE( STATUS )

string(ASCII 27 ESCAPE)
SET( ERROR   "${ESCAPE}[31m" )
SET( SUCCESS "${ESCAPE}[32m" )
SET( WARNING "${ESCAPE}[33m" )
SET( RESET   "${ESCAPE}[0m" )

INCLUDE( cmake/CMakeLists.txt )

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
SET(CMAKE_INSTALL_PREFIX
  "${CLSQUARE_PACKAGE_SOURCE_DIR}" CACHE PATH "CLSQUARE install prefix" FORCE
)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -w")

OPTION( TOOLS "Set to OFF to prevent compilation of tools." ON )

# if no option is given, standard is release
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

MESSAGE( STATUS "-- CLSquare package options ---------------------------------------------------" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX   = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "TOOLS                  = ${TOOLS}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE       = ${CMAKE_BUILD_TYPE}")
MESSAGE( STATUS "Change a value with: cmake -D<VAR>=<VALUE>" )
MESSAGE( STATUS )

SET( TOOLS "${TOOLS}" CACHE BOOL "Set to OFF to prevent compilation of tools." FORCE )

add_subdirectory(src)
add_subdirectory(demos)

IF ( TOOLS )
	add_subdirectory(tools)
ENDIF()

MESSAGE( STATUS "-- Installation summary -------------------------------------------------------" )
IF ( ANY_MODULE_MISSING )
  MESSAGE ( STATUS "${ERROR}ATTENTION: Some modules were not built because of missing dependencies.${RESET}" )
  MESSAGE ( STATUS "${ERROR}Please check the listings above whether everything you need was installed.${RESET}" )
ELSE()
  MESSAGE ( STATUS "${SUCCESS}All modules successfully installed.${RESET}" )
ENDIF()

MESSAGE ( STATUS "${SUCCESS}Please run the following command to set up the necessary environment variables${RESET}" )
MESSAGE ( STATUS "${SUCCESS}or put this in your bashrc. ${RESET}" )
MESSAGE ( STATUS "${SUCCESS}              source ${CMAKE_INSTALL_PREFIX}/bin/setup_env.bash ${RESET}" )

# doc target
IF(DOXYGEN_FOUND)
  MESSAGE(STATUS "${SUCCESS}You can generate the documentation by typing \"make doc\".${RESET}")
  ADD_CUSTOM_TARGET(doc COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE} 
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ELSE()
  MESSAGE(STATUS "${WARNING}Please install Doxygen if you would like to generate the documentation.${RESET}")
ENDIF()

MESSAGE ( STATUS "" )

FILE(WRITE ${CMAKE_INSTALL_PREFIX}/bin/setup_env.bash "export CLSQUARE_DIR=${CMAKE_INSTALL_PREFIX}\nexport CLSQUARE_BINARY_DIR=$CLSQUARE_DIR/bin\nexport CLSQUARE_BIN=$CLSQUARE_DIR/bin/clsquare\nexport PATH=$CLSQUARE_BINARY_DIR:$PATH\n") 
