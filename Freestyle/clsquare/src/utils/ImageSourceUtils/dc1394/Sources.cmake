CHECK_EXTERNAL ( DC1394Cam DC1394 )
CHECK_EXTERNAL ( DC1394Cam OpenCV)

IF ( NOT MISSING_DEPENDENCY )
  include_directories ( ${CURRENT_LIST_DIR} )
  LIST ( APPEND utils_srcs ${CURRENT_LIST_DIR}/dc1394cam.cpp )
ELSE ()
  set ( ANY_MISSING_IMGSOURCE TRUE )
ENDIF ()