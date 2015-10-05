CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

project(TAPIR CXX)

# keep cmake from screwing up the rpath
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# set paths for use by compiler
ADD_DEFINITIONS ( -DCLS_SRC_DIR=${TAPIR_SOURCE_DIR} )
ADD_DEFINITIONS ( -DCLS_LIB_DIR=${CMAKE_INSTALL_PREFIX}/lib )
ADD_DEFINITIONS ( -DCLS_BIN_DIR=${CMAKE_INSTALL_PREFIX}/bin )
ADD_DEFINITIONS ( -DCLS_EXE=${CMAKE_INSTALL_PREFIX}/bin/tapir )

# directories
INCLUDE_DIRECTORIES ( ${CMAKE_INSTALL_PREFIX}/include )
LINK_DIRECTORIES ( ${CMAKE_INSTALL_PREFIX}/lib )

# macro for checking external libraries
MACRO ( CHECK_EXTERNAL MODULE_NAME LIBRARY_NAME )
  IF ( NOT MISSING_DEPENDENCY AND NOT FOUND_${LIBRARY_NAME} )
    IF ( ${ARGN} MATCHES ".*OPTIONAL.*" )
      MESSAGE ( STATUS "building module ${MODULE_NAME} without dependency ${LIBRARY_NAME}; functionality may be reduced" )
    ELSE ()
      SET ( MISSING_DEPENDENCY TRUE )
      SET ( MISSING_DEPENDENCY TRUE PARENT_SCOPE )
      MESSAGE ( STATUS "skipping module ${MODULE_NAME} because dependency ${LIBRARY_NAME} is missing" )
      SET ( WITHLIB_${MODULE_NAME} OFF PARENT_SCOPE ) 
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# macro for checking internal libraries
MACRO ( CHECK_INTERNAL MODULE_NAME LIBRARY_NAME )
  IF ( NOT MISSING_DEPENDENCY AND NOT WITHLIB_${LIBRARY_NAME} )
    IF ( ${ARGN} MATCHES ".*OPTIONAL.*" )
      MESSAGE ( STATUS "building module ${MODULE_NAME} without library ${LIBRARY_NAME}; functionality may be reduced" )
    ELSE ()
      SET ( MISSING_DEPENDENCY TRUE )
      SET ( MISSING_DEPENDENCY TRUE PARENT_SCOPE )
      MESSAGE ( STATUS "skipping module ${MODULE_NAME} because library ${LIBRARY_NAME} was not compiled" )
      SET ( WITHLIB_${MODULE_NAME} OFF PARENT_SCOPE ) 
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# if no option is given, standard is release
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_RELEASE "-DTAPIR -Wall -O2 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-DTAPIR -Wall -g -fopenmp")

LINK_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/lib)

### Install OpenCV
# treated differently from other libs since it's an obligatory part
MESSAGE ( STATUS "-- Tapir library options ---------------------------------------------------" )
IF ( NOT DEFINED OPENCV_INSTALL_TYPE )
  SET ( OPENCV_INSTALL_TYPE TAR )
ENDIF ()
INCLUDE ( ${TAPIR_SOURCE_DIR}/kernel/opencv.cmake )
SET ( CURRENT_LIB_ENTRY OPENCV_INSTALL_TYPE )
STRING_PAD ( CURRENT_LIB_ENTRY 22 )
SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} = ${OPENCV_INSTALL_TYPE}" )
STRING_PAD ( CURRENT_LIB_ENTRY 30 )
MESSAGE ( STATUS "${CURRENT_LIB_ENTRY} Set to TAR for stable archive, GIT for newest")
SET ( CURRENT_LIB_ENTRY " " )
STRING_PAD ( CURRENT_LIB_ENTRY 30 )
MESSAGE ( STATUS "${CURRENT_LIB_ENTRY} source, or SYS for package manager's choice" )

### Print library listing
file( GLOB_RECURSE options ${TAPIR_SOURCE_DIR}/libs*/CMakeOptions.txt )
FOREACH( option ${options} )
  SET ( CURRENT_LIB_NAME "" )
  SET ( CURRENT_LIB_TEXT "" )
  SET ( CURRENT_LIB_DEFAULT OFF )
  INCLUDE ( ${option} )
  OPTION( WITHLIB_${CURRENT_LIB_NAME} ${CURRENT_LIB_TEXT} ${CURRENT_LIB_DEFAULT} )
  SET ( CURRENT_LIB_ENTRY WITHLIB_${CURRENT_LIB_NAME} )
  STRING_PAD ( CURRENT_LIB_ENTRY 22 )
  SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} = ${WITHLIB_${CURRENT_LIB_NAME}}" )
  STRING_PAD ( CURRENT_LIB_ENTRY 30 )
  IF ( ${WITHLIB_${CURRENT_LIB_NAME}} STREQUAL ON )
    SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} Set to OFF to disable" )
  ELSE ()
    SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} Set to ON to enable" )
  ENDIF ()
  MESSAGE( STATUS "${CURRENT_LIB_ENTRY} ${CURRENT_LIB_TEXT}" )
  # SET( WITHLIB_${CURRENT_LIB_NAME} "${WITHLIB_${CURRENT_LIB_NAME}}" ${CURRENT_LIB_TEXT} FORCE )
  IF ( WITHLIB_${CURRENT_LIB_NAME} )
    GET_FILENAME_COMPONENT ( CURRENT_LIST_DIR ${option} PATH )
    include_directories ( ${CURRENT_LIST_DIR} )
    include_directories ( ${CURRENT_LIST_DIR}/include )
    include_directories ( ${CURRENT_LIST_DIR}/src )
  ENDIF ()
ENDFOREACH( ${option} )
MESSAGE( STATUS "Change a value with: cmake -D<VAR>=<VALUE>" )
MESSAGE( STATUS )

### Build libraries
MESSAGE( STATUS "-- Building internal libraries ------------------------------------------------" )
file(GLOB_RECURSE libs ${TAPIR_SOURCE_DIR}/libs*/CMakeLists.txt)
FOREACH(lib ${libs})
  # go through libraries
  GET_FILENAME_COMPONENT(CURRENT_LIST_DIR ${lib} PATH)
  SET ( CURRENT_LIB_NAME "" )
  SET ( CURRENT_LIB_LINK ON )
  SET ( CURRENT_LIB_EXTERNAL "" )
  INCLUDE ( ${CURRENT_LIST_DIR}/CMakeOptions.txt )
  # try to build
  IF ( WITHLIB_${CURRENT_LIB_NAME} )
    set ( MISSING_DEPENDENCY FALSE )
    set ( TAPIR_INCLUDE_DIRS "" )
    # NOTE: while it would be nice to use include() here instead of add_subdirectory(),
    #       we can't do that, since then every file in the library would have to be
    #       prefixed with ${CURRENT_LIST_DIR}; that would be a lot of work for
    #       existing libraries, and impossible for external projects
    add_subdirectory ( ${CURRENT_LIST_DIR} )
    # if built successfully, link with it
    IF ( NOT MISSING_DEPENDENCY )
      IF ( CURRENT_LIB_LINK )
        list ( APPEND TAPIR_LIB_INTERNAL ${CURRENT_LIB_NAME} )
      ENDIF ()
      include_directories ( ${TAPIR_INCLUDE_DIRS} )
      message ( STATUS "building library ${CURRENT_LIB_NAME}" )
      list ( APPEND EXTERNAL_PROJECTS ${CURRENT_LIB_EXTERNAL} )
    ENDIF ()
  ENDIF ()
ENDFOREACH ()
MESSAGE( STATUS )

### Add kernel sources
include(${TAPIR_SOURCE_DIR}/kernel/Sources.cmake)

### Add module sources
MESSAGE( STATUS "-- Building modules -----------------------------------------------------------" )
MESSAGE( STATUS "all modules will be built unless noted otherwise" )
set ( module_types detector display imgsource output tracker )
foreach ( type ${module_types} )
  file ( GLOB_RECURSE ${type} ${TAPIR_SOURCE_DIR}/${type}*/Sources.cmake )
  foreach ( module ${${type}} )
    set ( MISSING_DEPENDENCY FALSE )
    get_filename_component ( CURRENT_LIST_DIR ${module} PATH )
    include ( ${module} )
  endforeach ()
endforeach ()
MESSAGE( STATUS )

# define subgroups for XCode and other IDEs
source_group( Kernel  FILES ${kernel_headers} ${kernel_srcs} )
source_group( Modules FILES ${module_headers} ${module_srcs} )

### Add utils sources
#include(${TAPIR_SOURCE_DIR}/utils/Sources.cmake)

# define the Tapir executable
add_executable ( tapir ${kernel_srcs}  ${kernel_headers} ${module_srcs} ${module_headers} )

# expand definitions
FOREACH ( definition ${TAPIR_LIB_DEFINITIONS} )
  ADD_DEFINITIONS ( ${definition} )
ENDFOREACH ()

# ensure that external libs are built in time
FOREACH ( project ${EXTERNAL_PROJECTS} )
  ADD_DEPENDENCIES ( tapir ${project} )
ENDFOREACH ()

# preprocess the dependencies
#list(REMOVE_DUPLICATES TAPIR_LIB_INTERNAL)
#list(REMOVE_DUPLICATES TAPIR_LIB_EXTERNAL)

# display status message for dependencies
#MESSAGE( STATUS "-- Dependencies ---------------------------------------------------------------" )
#MESSAGE( STATUS "linking with generated libs TAPIR_LIB_INTERNAL  = ${TAPIR_LIB_INTERNAL}" )
#MESSAGE( STATUS "linking with standard libs TAPIR_LIB_EXTERNAL   = ${TAPIR_LIB_EXTERNAL}" )
#MESSAGE( STATUS "including additional headers TAPIR_INCLUDE_DIRS = ${TAPIR_INCLUDE_DIRS}" )
#MESSAGE( STATUS )

target_link_libraries ( tapir ${TAPIR_LIB_INTERNAL} ${TAPIR_LIB_EXTERNAL} )

INSTALL ( TARGETS tapir 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

