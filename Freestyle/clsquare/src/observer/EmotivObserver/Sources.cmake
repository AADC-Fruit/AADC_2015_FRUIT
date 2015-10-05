CHECK_INTERNAL ( EmotivObserver Emotiv )

IF ( NOT MISSING_DEPENDENCY )


  LIST(APPEND observer_srcs 
    ${CURRENT_LIST_DIR}/emotivobserver.cpp
    ${CURRENT_LIST_DIR}/processedeog.cpp
  )

  CHECK_EXTERNAL( EOGObserver boost_thread )
  CHECK_EXTERNAL( EOGObserver Eigen3 )
  IF ( NOT MISSING_DEPENDENCY )
    LIST(APPEND observer_srcs 
      ${CURRENT_LIST_DIR}/eogobserver.cpp
      ${CURRENT_LIST_DIR}/eog2tapir.cpp
      ${CURRENT_LIST_DIR}/kukabinary.cpp
    )
    SET ( MISSING_DEPENDENCY FALSE PARENT_SCOPE )
  ENDIF ()
ENDIF ()
