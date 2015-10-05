### find optional packages
# 2.8 only, or did old ones have these as well?

# Variable naming conventions:
# For every library that was found, a variable FOUND_<lib> should be defined,
# with <lib> being the name that is passed to the linker, such as "boost" for
# libboost (as opposed to uppercase "Boost", which is set by the Find script).
# There should also be a compile definition of the form -DFOUND_<lib> of the
# same style, i.e. "-DFOUND_boost", though with any colons and hyphens replaced
# by an underscore. Use the FIND_ALIAS macro to automatically set both.
#
# General procedure:
#  FIND_PACKAGE( NAME1 )       # with NAME1 as in the name of the script
#                              # FindNAME1.cmake
#  FIND_RESULT ( NAME2 )       # with NAME2 as in the variable FOUND_NAME2 or
#                              # NAME2_FOUND set by the script
#  FIND_ALIAS  ( NAME2 ALIAS ) # with ALIAS being an arbitrary alternate name
#                              # you want to use
#  IF ( FOUND_NAME2 )
#    INCLUDE_DIRECTORIES ( ${INCLUDEPATH} )
#    LIST ( APPEND TAPIR_LIB_EXTERNAL ${LIBPATH} )
#  ENDIF ()                    # with INCLUDEPATH and LIBPATH being the ones
#                              # set by the find script, usually
#                              # NAME2_INCLUDE_DIR and NAME2_LIBRARY

### utility macro for output
MACRO ( FIND_RESULT LIB )
  IF ( ${LIB}_FOUND )
    SET ( FOUND_${LIB} TRUE )
  ENDIF ()
  IF ( SUPPRESS_${LIB} )
    MESSAGE ( STATUS "actively suppressing ${LIB}" )
    SET ( FOUND_${LIB} FALSE )
  ELSEIF ( FOUND_${LIB} )
    FIND_DEFINITION ( ${LIB} )
    MESSAGE ( STATUS "found ${LIB}" )
  ELSE ()
    MESSAGE ( STATUS "could NOT find ${LIB}" )
  ENDIF ()
ENDMACRO ()

### utility macro for setting aliases
MACRO ( FIND_ALIAS LIB ALIAS )
  SET ( FOUND_${ALIAS} ${FOUND_${LIB}} )
  FIND_DEFINITION ( ${ALIAS} )
ENDMACRO ()

### utility macro to set definitions
MACRO ( FIND_DEFINITION NAME )
  IF ( FOUND_${NAME} )
    STRING ( REPLACE "." "_" TEMP_STR ${NAME} )
    STRING ( REPLACE "-" "_" TEMP_STR ${TEMP_STR} )
    ADD_DEFINITIONS ( -DFOUND_${TEMP_STR} )
  ENDIF ()
ENDMACRO ()

MESSAGE ( STATUS "-- Searching for additional packages ------------------------------------------" )

SET ( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/registry ${CMAKE_MODULE_PATH} )

### included in standard build environment, no check needed
LIST ( APPEND TAPIR_LIB_EXTERNAL "pthread" )

### included with cmake by default

FIND_PACKAGE( X11 )
FIND_RESULT ( X11 )
FIND_ALIAS  ( X11 x11 )
IF ( FOUND_X11 )
  INCLUDE_DIRECTORIES ( ${X11_INCLUDE_DIR} )
  LIST ( APPEND TAPIR_LIB_EXTERNAL ${X11_LIBRARIES} )
ENDIF ()

### local scripts
# TODO libSM libICE libXext libm

FIND_PACKAGE( DC1394 )
FIND_RESULT ( DC1394 )
FIND_ALIAS  ( DC1394 dc1394 )
IF ( FOUND_DC1394 )
  INCLUDE_DIRECTORIES ( ${DC1394_INCLUDE_DIR} )
  LIST ( APPEND TAPIR_LIB_EXTERNAL ${DC1394_LIBRARY} )
ENDIF ()

SET ( FLTK_SKIP_FLUID TRUE )
SET ( FLTK_SKIP_OPENGL TRUE )
FIND_PACKAGE( FLTK )
FIND_RESULT ( FLTK )
FIND_ALIAS  ( FLTK fltk )
IF ( FOUND_FLTK )
  INCLUDE_DIRECTORIES ( ${FLTK_INCLUDE_DIR}/.. )
  LIST ( APPEND TAPIR_LIB_EXTERNAL ${FLTK_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( V4L )
FIND_RESULT ( V4L2 )
FIND_ALIAS  ( V4L2 V4L )
FIND_ALIAS  ( V4L2 Video4Linux )
if ( V4L2_FOUND )
  INCLUDE_DIRECTORIES ( ${V4L2_INCLUDE_DIR}/.. )
ENDIF ()

FIND_PACKAGE( Subversion )
FIND_RESULT ( Subversion )

MESSAGE ( STATUS )
