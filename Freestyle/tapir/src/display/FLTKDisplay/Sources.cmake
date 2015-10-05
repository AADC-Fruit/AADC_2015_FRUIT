CHECK_EXTERNAL ( FLTKDisplay FLTK )

IF ( NOT MISSING_DEPENDENCY )
  include_directories ( ${CURRENT_LIST_DIR} )
  LIST ( APPEND TAPIR_LIB_EXTERNAL ${FLTK_LIBRARIES} )
  LIST ( APPEND module_srcs
    ${CURRENT_LIST_DIR}/Fl_cvDisplay.cpp
    ${CURRENT_LIST_DIR}/fltkdisplay.cpp
  )
ENDIF ()
