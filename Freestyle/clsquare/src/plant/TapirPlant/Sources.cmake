CHECK_INTERNAL ( TapirPlant Tapir )

IF ( NOT MISSING_DEPENDENCY )


LIST(APPEND plant_srcs 
	${CURRENT_LIST_DIR}/tapirplant.cc
)

ENDIF ()
