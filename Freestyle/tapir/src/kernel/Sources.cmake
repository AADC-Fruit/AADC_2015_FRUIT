include_directories(${TAPIR_SOURCE_DIR}/kernel)
LIST(APPEND kernel_srcs 
	${TAPIR_SOURCE_DIR}/kernel/mainloop.cpp
	${TAPIR_SOURCE_DIR}/kernel/registry.cpp
	${TAPIR_SOURCE_DIR}/kernel/valueparser.cpp
	${TAPIR_SOURCE_DIR}/kernel/pipe.cpp
) 

LIST(APPEND kernel_headers
  ${TAPIR_SOURCE_DIR}/kernel/imagesource.h
  ${TAPIR_SOURCE_DIR}/kernel/detector.h
  ${TAPIR_SOURCE_DIR}/kernel/tracker.h
  ${TAPIR_SOURCE_DIR}/kernel/output.h
  ${TAPIR_SOURCE_DIR}/kernel/display.h
  ${TAPIR_SOURCE_DIR}/kernel/global.h
  ${TAPIR_SOURCE_DIR}/kernel/registry.h
  ${TAPIR_SOURCE_DIR}/kernel/valueparser.h
	${TAPIR_SOURCE_DIR}/kernel/pipe.h
)
