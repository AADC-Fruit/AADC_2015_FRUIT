CHECK_EXTERNAL ( DefaultSources OpenCV)

IF ( NOT MISSING_DEPENDENCY )
  include_directories(${CURRENT_LIST_DIR})
  LIST(APPEND utils_srcs 
	  ${CURRENT_LIST_DIR}/filesource.cpp
	  ${CURRENT_LIST_DIR}/camsource.cpp
  ) 

ELSE ()

  set ( ANY_MISSING_IMGSOURCE TRUE )
  
ENDIF ()
