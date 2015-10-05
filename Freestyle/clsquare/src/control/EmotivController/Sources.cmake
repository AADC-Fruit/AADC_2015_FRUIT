CHECK_INTERNAL ( EmotivControl Emotiv )

IF ( NOT MISSING_DEPENDENCY )
  LIST ( APPEND plant_srcs 
    ${CURRENT_LIST_DIR}/emotivcontrol.cpp
  )
ENDIF ()
