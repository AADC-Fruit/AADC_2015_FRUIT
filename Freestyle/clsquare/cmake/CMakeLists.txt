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
#    LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${LIBPATH} )
#  ENDIF ()                    # with INCLUDEPATH and LIBPATH being the ones
#                              # set by the find script, usually
#                              # NAME2_INCLUDE_DIR and NAME2_LIBRARY

### get all those find scripts to shut the hell up...
function ( message )
  list ( GET ARGV 0 MessageType )
  if ( MessageType STREQUAL FATAL_ERROR OR CMAKE_BUILD_TYPE STREQUAL Debug )
    list ( REMOVE_AT ARGV 0 )
    _message ( ${MessageType} "${ARGV}" )
  endif ()
endfunction ()
### ...and make a backup copy of the original function
function ( _message )
  list ( GET ARGV 0 MessageType )
  list ( REMOVE_AT ARGV 0 )
  __message ( ${MessageType} "${ARGV}" )
endfunction ()

### utility macro for output
MACRO ( FIND_RESULT LIB )
  IF ( ${LIB}_FOUND )
    SET ( FOUND_${LIB} TRUE )
  ENDIF ()
  IF ( SUPPRESS_${LIB} )
    _MESSAGE ( STATUS "${ESCAPE}[32mactively suppressing ${LIB}${ESCAPE}[0m" )
    SET ( FOUND_${LIB} FALSE )
  ELSEIF ( FOUND_${LIB} )
    FIND_DEFINITION ( ${LIB} )
    _MESSAGE ( STATUS "found ${LIB}" )
  ELSE ()
    IF ( NOT "${ARGN}" STREQUAL "" )
      SET ( HINT "(${ARGN})" )
    ELSE ()
      SET ( HINT "" )
    ENDIF ()
    _MESSAGE ( STATUS "${ESCAPE}[33mcould NOT find ${LIB}${ESCAPE}[0m ${HINT}" )
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
    STRING ( REPLACE "+" "_" TEMP_STR ${TEMP_STR} )
    ADD_DEFINITIONS ( -DFOUND_${TEMP_STR} )
  ENDIF ()
ENDMACRO ()

### utility macro to find libraries that do not have their own Find-script
MACRO ( FIND_GENERIC NAME HEADER LIB )
  SET ( GENERIC_ID ${NAME} )
  SET ( GENERIC_HEADER ${HEADER} )
  SET ( GENERIC_LIB ${LIB} )
  FIND_PACKAGE ( Generic )
ENDMACRO ()

_MESSAGE ( STATUS "-- Searching for additional packages ------------------------------------------" )

SET ( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

### included in standard build environment, no check needed
# LIST ( APPEND CLSQUARE_LIB_EXTERNAL "pthread" )

### included with cmake by default

FIND_PACKAGE( Doxygen )
FIND_RESULT ( DOXYGEN )

FIND_PACKAGE( Subversion )
FIND_RESULT ( Subversion )

FIND_PACKAGE( Boost COMPONENTS system regex thread serialization )
FIND_RESULT ( Boost )
FIND_ALIAS  ( Boost boost )

MACRO ( BOOST_MODULE_CHECK UPPER LOWER )
  FIND_RESULT ( Boost_${UPPER} )
  FIND_ALIAS  ( Boost_${UPPER} boost_${LOWER} )
  IF ( FOUND_Boost_${UPPER} )
    INCLUDE_DIRECTORIES ( ${BOOST_INCLUDEDIR} )
    LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${Boost_${UPPER}_LIBRARY} )
  ENDIF ()
ENDMACRO ()

BOOST_MODULE_CHECK ( THREAD thread )
BOOST_MODULE_CHECK ( DETAIL detail )
BOOST_MODULE_CHECK ( SYSTEM system )
BOOST_MODULE_CHECK ( REGEX regex )
BOOST_MODULE_CHECK ( SERIALIZATION serialization )

# FIND_PACKAGE( GTest )
# FIND_RESULT ( GTEST )
# FIND_ALIAS  ( GTEST GTest )
# IF ( FOUND_GTEST )
#   INCLUDE_DIRECTORIES ( ${GTEST_INCLUDE_DIR} )
#   LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${GTEST_BOTH_LIBRARIES} )
# ENDIF ()

FIND_PACKAGE( Protobuf )
FIND_RESULT ( PROTOBUF )
FIND_ALIAS  ( PROTOBUF Protobuf )
FIND_ALIAS  ( PROTOBUF protobuf )
IF ( FOUND_PROTOBUF )
  INCLUDE_DIRECTORIES ( ${PROTOBUF_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${PROTOBUF_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( X11 )
FIND_RESULT ( X11 )
FIND_ALIAS  ( X11 x11 )
IF ( FOUND_X11 )
  INCLUDE_DIRECTORIES ( ${X11_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${X11_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( Threads )
FIND_RESULT ( Threads )
IF ( FOUND_Threads )
  IF ( CMAKE_USE_PTHREADS_INIT )
    FIND_ALIAS ( Threads pthread )
  ENDIF ()
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${CMAKE_THREAD_LIBS_INIT} )
ENDIF ()

### local scripts

FIND_PACKAGE( ANN )
FIND_RESULT ( ANN )
IF ( FOUND_ANN )
  INCLUDE_DIRECTORIES ( ${ANN_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${ANN_LIBRARY} )
ENDIF ()

# BLAS also has a default script in 2.8, but that one does not always work
FIND_PACKAGE( BLAS )
FIND_RESULT ( BLAS )
FIND_ALIAS  ( BLAS blas )
IF ( FOUND_BLAS )
  INCLUDE_DIRECTORIES ( ${BLAS_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${BLAS_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( CBLAS )
FIND_RESULT ( CBLAS )
FIND_ALIAS  ( CBLAS cblas )
IF ( FOUND_CBLAS )
  INCLUDE_DIRECTORIES ( ${CBLAS_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${CBLAS_LIBRARY} )
ENDIF ()

FIND_PACKAGE( DC1394 )
FIND_RESULT ( DC1394 )
FIND_ALIAS  ( DC1394 dc1394 )
IF ( FOUND_DC1394 )
  __message ( STATUS "${WARNING}Compiling against libdc1394 may cause crashes. If you experience problems, re-run CMake with -DSUPPRESS_DC1394=TRUE.${RESET}" )
  INCLUDE_DIRECTORIES ( ${DC1394_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${DC1394_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Eigen3 )
FIND_RESULT ( EIGEN3 )
FIND_ALIAS  ( EIGEN3 Eigen3 )
IF ( FOUND_EIGEN3 )
  INCLUDE_DIRECTORIES ( ${EIGEN3_INCLUDE_DIR} )
  # Eigen3 is header-only
ENDIF ()

FIND_PACKAGE( FTD2XX )
FIND_RESULT ( FTD2XX "download from http://www.ftdichip.com/Drivers/D2XX.htm" )
FIND_ALIAS  ( FTD2XX ftd2xx )
IF ( FOUND_FTD2XX )
  INCLUDE_DIRECTORIES ( ${FTD2XX_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${FTD2XX_LIBRARY} )
ENDIF ()

FIND_PACKAGE( FTDI )
FIND_RESULT ( FTDI )
FIND_ALIAS  ( FTDI ftdi )
IF ( FOUND_FTDI )
  INCLUDE_DIRECTORIES ( ${FTDI_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${FTDI_LIBRARY} )
ENDIF ()

FIND_PACKAGE( NFQCA )
FIND_RESULT ( NFQCA "try specifying its path using -DNFQCA_DIR=<...>" )
FIND_ALIAS  ( NFQCA nfqca )
IF ( FOUND_NFQCA )
  INCLUDE_DIRECTORIES ( ${NFQCA_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${NFQCAActor_LIBRARY} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${NFQCACritic_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Mono2 )
FIND_RESULT ( MONO2 )
FIND_ALIAS  ( MONO2 mono-2.0 )
IF ( FOUND_MONO2 )
  INCLUDE_DIRECTORIES ( ${MONO2_INCLUDE_DIR}/mono-2.0 )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${MONO2_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Plotter )
FIND_RESULT ( PLOTTER "install libplot-dev" )
FIND_ALIAS  ( PLOTTER plotter )
IF ( FOUND_PLOTTER )
  INCLUDE_DIRECTORIES ( ${PLOTTER_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${PLOTTER_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( PCan )
FIND_RESULT ( PCAN "download from http://www.peak-system.com/fileadmin/media/linux/index.htm")
FIND_ALIAS  ( PCAN pcan )
IF ( FOUND_PCAN )
  INCLUDE_DIRECTORIES ( ${PCAN_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${PCAN_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Usb )
FIND_RESULT ( USB "install libusb-dev")
FIND_ALIAS  ( USB usb )
FIND_ALIAS  ( USB usb-0.1 )
IF ( FOUND_USB )
  INCLUDE_DIRECTORIES ( ${USB_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${USB_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Usb-1.0 )
FIND_RESULT ( USB_1 "install libusb-1.0-dev")
FIND_ALIAS  ( USB_1 usb-1.0 )
IF ( FOUND_USB_1 )
  INCLUDE_DIRECTORIES ( ${USB_1_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${USB_1_LIBRARY} )
ENDIF ()

FIND_PACKAGE( Joystick )
FIND_RESULT ( JOYSTICK )
FIND_ALIAS  ( JOYSTICK Joystick )
IF ( FOUND_JOYSTICK )
  INCLUDE_DIRECTORIES ( ${JOYSTICK_INCLUDE_DIR} )
ENDIF ()

FIND_PACKAGE( HIDAPI )
FIND_RESULT ( HIDAPI "download from http://www.signal11.us/oss/hidapi/" )
FIND_ALIAS  ( HIDAPI hidapi )
IF ( FOUND_HIDAPI )
  INCLUDE_DIRECTORIES ( ${HIDAPI_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${HIDAPI_LIBRARIES} )
  LINK_DIRECTORIES(/usr/local/lib)
ENDIF ()

FIND_PACKAGE( Mcrypt )
FIND_RESULT ( Mcrypt "install libmcrypt-dev")
FIND_ALIAS  ( Mcrypt MCrypt )
IF ( FOUND_Mcrypt )
  INCLUDE_DIRECTORIES ( ${Mcrypt_INCLUDE_DIR} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${Mcrypt_LIBS} )
ENDIF ()

FIND_PACKAGE( ImageMagick COMPONENTS Magick++)
FIND_RESULT ( ImageMagick_Magick++ "install libmagick++-dev" )
FIND_ALIAS  ( ImageMagick_Magick++ Magick++ )
FIND_ALIAS  ( ImageMagick_Magick++ ImageMagick )
IF ( FOUND_ImageMagick_Magick++ )
  INCLUDE_DIRECTORIES ( ${ImageMagick_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${ImageMagick_LIBRARIES} )
ENDIF ()

SET ( FLTK_SKIP_FLUID TRUE )
SET ( FLTK_SKIP_OPENGL TRUE )
FIND_PACKAGE( FLTK )
FIND_RESULT ( FLTK "install libfltk1.3-dev")
FIND_ALIAS  ( FLTK fltk )
IF ( FOUND_FLTK )
  INCLUDE_DIRECTORIES ( ${FLTK_INCLUDE_DIR}/.. )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${FLTK_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( GSL )
FIND_RESULT ( GSL )
FIND_ALIAS  ( GSL gsl )
IF ( FOUND_GSL )
  INCLUDE_DIRECTORIES ( ${GSL_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${GSL_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( V4L )
FIND_RESULT ( V4L2 )
FIND_ALIAS  ( V4L2 V4L )
FIND_ALIAS  ( V4L2 Video4Linux )
if ( FOUND_V4L2 )
  INCLUDE_DIRECTORIES ( ${V4L2_INCLUDE_DIR}/.. )
ENDIF ()

FIND_PACKAGE( OpenCV2 )
FIND_RESULT ( OpenCV2 )
FIND_ALIAS  ( OpenCV2 OpenCV )
if ( FOUND_OpenCV )
  INCLUDE_DIRECTORIES ( ${OpenCV2_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${OpenCV2_LIBRARIES} )
ENDIF ()

# since ROS sets the usual environment variables, this should be good enough
FIND_PACKAGE( ROS )
FIND_RESULT ( ROS "install ros-hydro-desktop-all or make sure that \$ROS_ROOT is set" )
FIND_ALIAS  ( ROS ros )
IF ( FOUND_ROS )
  INCLUDE_DIRECTORIES ( ${ROS_INCLUDE_DIRS} )
  LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${ROS_LIBRARIES} )
ENDIF ()

FIND_PACKAGE( ROSCLS )
FIND_RESULT ( ROSCLS "try specifying its path using -DROSCLS_DIR=<...>" )
FIND_ALIAS  ( ROSCLS roscls )
IF ( FOUND_ROSCLS )
  INCLUDE_DIRECTORIES ( ${ROSCLS_INCLUDE_DIRS} )
ENDIF ()

### using generic script

# FIND_GENERIC( LEVELDB leveldb/c.h leveldb )
# FIND_RESULT ( LEVELDB )
# FIND_ALIAS  ( LEVELDB leveldb )
# IF ( FOUND_LEVELDB )
#   INCLUDE_DIRECTORIES ( ${LEVELDB_INCLUDE_DIRS} )
#   LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${LEVELDB_LIBRARIES} )
# ENDIF ()
# 
# FIND_GENERIC( SNAPPY snappy.h snappy )
# FIND_RESULT ( SNAPPY )
# FIND_ALIAS  ( SNAPPY snappy )
# IF ( FOUND_SNAPPY )
#   INCLUDE_DIRECTORIES ( ${SNAPPY_INCLUDE_DIRS} )
#   LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${SNAPPY_LIBRARIES} )
# ENDIF ()

# FIND_PACKAGE( CUDA )
# FIND_RESULT ( CUDA )
# IF ( FOUND_CUDA )
#   INCLUDE_DIRECTORIES ( ${CUDA_INCLUDE_DIRS} )
#   LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${CUDA_LIBRARIES} )
# ENDIF ()
# 
# # Caffe can only be used with CUDA and Protobuf, possibly others. It may not be linked to them yet, so ensure that they are found!
# IF ( FOUND_CUDA AND FOUND_Protobuf )
#   FIND_GENERIC( CAFFE caffe/caffe.hpp caffe )
#   FIND_RESULT ( CAFFE )
#   FIND_ALIAS  ( CAFFE Caffe )
#   IF ( FOUND_CAFFE )
#     INCLUDE_DIRECTORIES ( ${CAFFE_INCLUDE_DIRS} )
#     LIST ( APPEND CLSQUARE_LIB_EXTERNAL ${CAFFE_LIBRARIES} )
#   ENDIF ()
# ENDIF ()

### restore message function
function ( message )
  list ( GET ARGV 0 MessageType )
  list ( REMOVE_AT ARGV 0 )
  __message ( ${MessageType} "${ARGV}" )
endfunction ()

MESSAGE ( STATUS "To prevent a package from being linked into CLSquare, run cmake again with -DSUPPRESS_<lib>=TRUE" )

MESSAGE ( STATUS )
