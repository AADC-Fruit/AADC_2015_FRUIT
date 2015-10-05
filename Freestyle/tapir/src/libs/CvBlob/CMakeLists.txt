CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

CHECK_EXTERNAL ( cvblob Subversion )
IF ( NOT MISSING_DEPENDENCY )

  INCLUDE ( ExternalProject )

  ExternalProject_Add ( libcvblob
    SVN_REPOSITORY http://cvblob.googlecode.com/svn/trunk/
    SVN_REVISION -r "392"
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  # -DOpenCV_DIR=${CMAKE_BINARY_DIR}/src/lib/opencv/opencv-prefix/src/opencv-build
  )

  ADD_DEPENDENCIES ( libcvblob libopencv )
  #LIST ( APPEND TAPIR_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/cvblob/include )
  #SET ( TAPIR_INCLUDE_DIRS ${TAPIR_INCLUDE_DIRS} PARENT_SCOPE )

ENDIF ()
