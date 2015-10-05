CHECK_EXTERNAL ( V4L2ImageSource V4L )
CHECK_EXTERNAL ( V4L2ImageSource OpenCV )

IF ( NOT MISSING_DEPENDENCY )
  include_directories ( ${CURRENT_LIST_DIR} )
  LIST ( APPEND utils_srcs
    ${CURRENT_LIST_DIR}/V4L2ImageSource.cpp
  )
ELSE ()
  set ( ANY_MISSING_IMGSOURCE TRUE )
ENDIF ()
