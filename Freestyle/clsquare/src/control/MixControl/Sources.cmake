INCLUDE_DIRECTORIES(${CURRENT_LIST_DIR})

LIST(APPEND controller_srcs 
	${CURRENT_LIST_DIR}/mixcontrol.cpp
	${CURRENT_LIST_DIR}/operation.cpp
	${CURRENT_LIST_DIR}/operations.cpp
)
