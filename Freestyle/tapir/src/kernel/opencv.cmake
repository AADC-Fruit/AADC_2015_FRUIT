CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

IF ( OPENCV_INSTALL_TYPE STREQUAL "SYS" )

  FIND_PACKAGE ( OpenCV REQUIRED )
  IF ( FOUND_OpenCV OR OpenCV_FOUND )
    include_directories ( ${OpenCV_INCLUDE_DIR} )
    link_directories ( ${OpenCV_LIBRARY_DIR} )
  ENDIF ()

  # dummy target for install order
  add_custom_target ( libopencv )

  list ( APPEND TAPIR_LIB_EXTERNAL "opencv_core" )
  list ( APPEND TAPIR_LIB_EXTERNAL "opencv_highgui" )
  list ( APPEND TAPIR_LIB_EXTERNAL "opencv_imgproc" )
  list ( APPEND TAPIR_LIB_EXTERNAL "opencv_objdetect" )

ELSE ()

  INCLUDE ( ExternalProject )
  IF ( OPENCV_INSTALL_TYPE STREQUAL GIT )
    ExternalProject_Add ( libopencv
      GIT_REPOSITORY git://code.opencv.org/opencv.git
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      -DBUILD_TESTS=OFF
    )
  ELSE ()
    ExternalProject_Add ( libopencv
      URL http://ml.informatik.uni-freiburg.de/_media/downloads/opencv-2.4.2.tar.gz
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      -DBUILD_TESTS=OFF
    )
  ENDIF ()

  LIST ( APPEND TAPIR_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/opencv/include )
  SET ( TAPIR_INCLUDE_DIRS ${TAPIR_INCLUDE_DIRS} PARENT_SCOPE )
  include_directories ( ${CMAKE_INSTALL_PREFIX}/lib/OpenCV/include )
  link_directories ( ${CMAKE_INSTALL_PREFIX}/lib/OpenCV/lib )

  list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  list(APPEND TAPIR_LIB_INTERNAL "opencv_core")
  list(APPEND TAPIR_LIB_INTERNAL "opencv_highgui")
  list(APPEND TAPIR_LIB_INTERNAL "opencv_imgproc")
  list(APPEND TAPIR_LIB_INTERNAL "opencv_objdetect")

ENDIF()

