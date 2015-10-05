CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3)

project(CLSQUARE CXX)

# keep cmake from screwing up the rpath
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# set paths for use by compiler
ADD_DEFINITIONS ( -DCLS_SRC_DIR=${CLSQUARE_SOURCE_DIR} )
ADD_DEFINITIONS ( -DCLS_LIB_DIR=${CMAKE_INSTALL_PREFIX}/lib )
ADD_DEFINITIONS ( -DCLS_BIN_DIR=${CMAKE_INSTALL_PREFIX}/bin )
ADD_DEFINITIONS ( -DCLS_EXE=${CMAKE_INSTALL_PREFIX}/bin/clsquare )

SET ( ANY_MODULE_MISSING FALSE )

# macro for checking external libraries
MACRO ( CHECK_EXTERNAL MODULE_NAME LIBRARY_NAME )
  IF ( NOT MISSING_DEPENDENCY AND NOT FOUND_${LIBRARY_NAME} )
    SET ( ANY_MODULE_MISSING TRUE )
    SET ( ANY_MODULE_MISSING TRUE PARENT_SCOPE )
    IF ( ${ARGN} MATCHES ".*OPTIONAL.*" )
      MESSAGE ( STATUS "${ESCAPE}[33mbuilding ${MODULE_NAME} without dependency ${LIBRARY_NAME}; functionality may be reduced${ESCAPE}[0m" )
    ELSE ()
      SET ( MISSING_DEPENDENCY TRUE )
      SET ( MISSING_DEPENDENCY TRUE PARENT_SCOPE )
      MESSAGE ( STATUS "${ESCAPE}[33mskipping ${MODULE_NAME} because dependency ${LIBRARY_NAME} is missing${ESCAPE}[0m" )
      SET ( WITHLIB_${MODULE_NAME} OFF PARENT_SCOPE ) 
    ENDIF ()
  ENDIF ()
ENDMACRO ()

# macro for checking internal libraries
MACRO ( CHECK_INTERNAL MODULE_NAME LIBRARY_NAME )
  IF ( NOT MISSING_DEPENDENCY AND NOT WITHLIB_${LIBRARY_NAME} )
    #SET ( ANY_MODULE_MISSING TRUE )
    #SET ( ANY_MODULE_MISSING TRUE PARENT_SCOPE )
    IF ( ${ARGN} MATCHES ".*OPTIONAL.*" )
      MESSAGE ( STATUS "${ESCAPE}[33mbuilding ${MODULE_NAME} without internal library ${LIBRARY_NAME}; functionality may be reduced${ESCAPE}[0m" )
    ELSE ()
      SET ( MISSING_DEPENDENCY TRUE )
      SET ( MISSING_DEPENDENCY TRUE PARENT_SCOPE )
      MESSAGE ( STATUS "${ESCAPE}[33mskipping ${MODULE_NAME} because internal library ${LIBRARY_NAME} was not compiled${ESCAPE}[0m" )
      SET ( WITHLIB_${MODULE_NAME} OFF PARENT_SCOPE ) 
    ENDIF ()
  ENDIF ()
ENDMACRO ()

set(CMAKE_CXX_FLAGS_RELEASE "-DCLSQUARE -Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-DCLSQUARE -Wall -g")

# OpenMP
FIND_PACKAGE( OpenMP )
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

LINK_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/lib)

### Print library listing
MESSAGE( STATUS "-- CLSquare library options ---------------------------------------------------" )
file( GLOB_RECURSE options ${CLSQUARE_SOURCE_DIR}/libs*/CMakeOptions.txt )
list( SORT options )
FOREACH( option ${options} )
  SET ( CURRENT_LIB_NAME "" )
  SET ( CURRENT_LIB_TEXT "" )
  SET ( CURRENT_LIB_DEFAULT OFF )
  INCLUDE ( ${option} )
  IF ( ALL_LIBS )
    SET ( CURRENT_LIB_DEFAULT ON )
  ENDIF()
  OPTION( WITHLIB_${CURRENT_LIB_NAME} ${CURRENT_LIB_TEXT} ${CURRENT_LIB_DEFAULT} )
  SET ( CURRENT_LIB_ENTRY WITHLIB_${CURRENT_LIB_NAME} )
  STRING_PAD ( CURRENT_LIB_ENTRY 22 )
  SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} = ${WITHLIB_${CURRENT_LIB_NAME}}" )
  STRING_PAD ( CURRENT_LIB_ENTRY 30 )
#  IF ( ${WITHLIB_${CURRENT_LIB_NAME}} STREQUAL ON )
#    SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} Set to OFF to disable" )
#  ELSE ()
#    SET ( CURRENT_LIB_ENTRY "${CURRENT_LIB_ENTRY} Set to ON to enable" )
#  ENDIF ()
  MESSAGE( STATUS "${CURRENT_LIB_ENTRY} ${CURRENT_LIB_TEXT}" )
  IF ( WITHLIB_${CURRENT_LIB_NAME} )
    GET_FILENAME_COMPONENT ( CURRENT_LIST_DIR ${option} PATH )
    include_directories ( ${CURRENT_LIST_DIR} )
    #include_directories ( ${CURRENT_LIST_DIR}/include )
    #include_directories ( ${CURRENT_LIST_DIR}/src )
  ENDIF ()
ENDFOREACH( ${option} )
MESSAGE( STATUS "Change a value with: cmake <path_to_cmakelist> -D<VAR>=<VALUE>" )
MESSAGE( STATUS )

#include_directories(${CMAKE_INSTALL_PREFIX}/include)

### Build libraries
MESSAGE( STATUS "-- Building internal libraries ------------------------------------------------" )
set(CLSQUARE_LIB_INTERNAL "")
set(CLSQUARE_VIRTUAL_DEPS "")
file(GLOB_RECURSE libs ${CLSQUARE_SOURCE_DIR}/libs*/CMakeLists.txt)
list( SORT libs )
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
    set ( CLSQUARE_INCLUDE_DIRS "" )
    # NOTE: while it would be nice to use include() here instead of add_subdirectory(),
    #       we can't do that, since then every file in the library would have to be
    #       prefixed with ${CURRENT_LIST_DIR}; that would be a lot of work for
    #       existing libraries, and impossible for external projects
    add_subdirectory ( ${CURRENT_LIST_DIR} )
    # if built successfully, link with it
    IF ( NOT MISSING_DEPENDENCY )
      IF ( CURRENT_LIB_LINK )
        list ( APPEND CLSQUARE_LIB_INTERNAL ${CURRENT_LIB_NAME} )
      ENDIF ()
      include_directories ( ${CLSQUARE_INCLUDE_DIRS} )
      message ( STATUS "building library ${CURRENT_LIB_NAME}" )
      list ( APPEND EXTERNAL_PROJECTS ${CURRENT_LIB_EXTERNAL} )
      # if the built lib was not linked, link dependencies instead
      IF ( NOT CURRENT_LIB_LINK )
        list ( APPEND CLSQUARE_VIRTUAL_DEPS ${CURRENT_LIB_NAME} )
        list ( APPEND CLSQUARE_LIB_INTERNAL ${CURRENT_LIB_EXTERNAL} )
      ENDIF ()
    ENDIF ()
  ENDIF ()
ENDFOREACH ()
MESSAGE( STATUS )

### Add kernel sources
include(${CLSQUARE_SOURCE_DIR}/kernel/Sources.cmake)

### Add module sources
MESSAGE( STATUS "-- Building modules -----------------------------------------------------------" )
MESSAGE( STATUS "all modules will be built unless noted otherwise" )
include_directories ( ${CLSQUARE_SOURCE_DIR} )
set ( module_types plant control reward graphic observer input output statistics )
foreach ( type ${module_types} )
  include_directories ( ${CLSQUARE_SOURCE_DIR}/${type} )
  file ( GLOB_RECURSE ${type}s ${CLSQUARE_SOURCE_DIR}/${type}*/Sources.cmake )
  list ( SORT ${type}s )
  foreach ( module ${${type}s} )
    set ( MISSING_DEPENDENCY FALSE )
    get_filename_component ( CURRENT_LIST_DIR ${module} PATH )
    include ( ${module} )
  endforeach ()
endforeach ()

### Add utils sources
include(${CLSQUARE_SOURCE_DIR}/utils/Sources.cmake)
MESSAGE( STATUS )

# define subgroups for XCode and other IDEs
source_group( Kernel FILES ${kernel_headers} ${kernel_srcs} )
source_group( Controllers FILES ${controller_headers} ${controller_srcs} )
source_group( Plants FILES ${plant_headers} ${plant_srcs} )
source_group( Reward FILES ${reward_headers} ${reward_srcs} )
source_group( Graphic FILES ${graphic_headers} ${graphic_srcs} )
source_group( Statistics FILES ${statistics_headers} ${statistics_srcs} )
source_group( Utils FILES ${utils_headers} ${utils_srcs} )
source_group( Observers FILES ${observer_headers} ${observer_srcs} )
source_group( Inputs FILES ${input_headers} ${input_srcs} )
source_group( Outputs FILES ${outpus_headers} ${output_srcs} )

# define the CLSquare executable
add_executable(clsquare ${kernel_srcs} ${plant_srcs} ${controller_srcs} ${reward_srcs} ${graphic_srcs} ${statistics_srcs} ${utils_srcs} ${observer_srcs} ${input_srcs} ${output_srcs}
                        ${kernel_headers} ${plant_headers} ${controller_headers} ${reward_headers} ${graphic_headers} ${statistics_headers} ${utils_headers} ${observer_headers} ${input_headers} ${output_headers})

# expand definitions
FOREACH ( definition ${CLSQUARE_LIB_DEFINITIONS} )
  ADD_DEFINITIONS ( ${definition} )
ENDFOREACH ()

# ensure that external libs are built in time
FOREACH ( project ${EXTERNAL_PROJECTS} )
  ADD_DEPENDENCIES ( clsquare ${project} )
ENDFOREACH ()
FOREACH ( project ${CLSQUARE_VIRTUAL_DEPS} )
  ADD_DEPENDENCIES ( clsquare lib${project} )
ENDFOREACH ()

# preprocess the dependencies
list(REMOVE_DUPLICATES CLSQUARE_LIB_INTERNAL)
list(REMOVE_DUPLICATES CLSQUARE_LIB_EXTERNAL)

# display status message for dependencies
# MESSAGE( STATUS "-- Dependencies ---------------------------------------------------------------" )
# MESSAGE( STATUS "linking with generated libs CLSQUARE_LIB_INTERNAL  = ${CLSQUARE_LIB_INTERNAL}" )
# MESSAGE( STATUS "linking with standard libs CLSQUARE_LIB_EXTERNAL   = ${CLSQUARE_LIB_EXTERNAL}" )
# MESSAGE( STATUS "including additional headers CLSQUARE_INCLUDE_DIRS = ${CLSQUARE_INCLUDE_DIRS}" )
# MESSAGE( STATUS )

target_link_libraries( clsquare ${CLSQUARE_LIB_INTERNAL} ${CLSQUARE_LIB_EXTERNAL} ${CMAKE_DL_LIBS} )

INSTALL(FILES ${kernel_headers} DESTINATION include)

INSTALL(TARGETS clsquare 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

SET ( ANY_MODULE_MISSING ${ANY_MODULE_MISSING} PARENT_SCOPE )
