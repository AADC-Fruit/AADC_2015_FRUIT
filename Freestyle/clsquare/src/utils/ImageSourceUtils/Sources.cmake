set ( MISSING_DEPENDENCY FALSE )
set ( ANY_MISSING_IMGSOURCE FALSE )

CHECK_EXTERNAL ( ImageSource-Utils OpenCV )
CHECK_EXTERNAL ( ImageSource-Utils dc1394 )

IF ( NOT MISSING_DEPENDENCY )

  file(GLOB_RECURSE imgsources ${CLSQUARE_SOURCE_DIR}/utils/ImageSourceUtils/*/Sources.cmake)
  FOREACH(imgsource ${imgsources})
    GET_FILENAME_COMPONENT(CURRENT_LIST_DIR ${imgsource} PATH)
    set ( MISSING_DEPENDENCY FALSE )
    include(${CURRENT_LIST_DIR}/Sources.cmake)
  ENDFOREACH()

ELSE ()

  set ( ANY_MISSING_IMGSOURCE TRUE )

ENDIF ()
