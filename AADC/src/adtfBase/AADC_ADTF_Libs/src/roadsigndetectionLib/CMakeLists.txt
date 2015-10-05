set(LIB_NAME trafficsignrecognition)

include_directories(.)
set(AADC_ROADSIGN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "includes for trafficsign lib")

#SET Source Files
set(SOURCE	
	algorithm/ihls_nhs/ihls.cpp
	algorithm/ihls_nhs/math_utils.cpp
	algorithm/ihls_nhs/nhs.cpp
	CRoadsignDetectionDLL.cpp
	cRoadSigns.cpp
	)

#SET Header Files
set(HEADER
	stdafx.h
	algorithm/ihls_nhs/ihls.h
	algorithm/ihls_nhs/math_utils.h
	algorithm/ihls_nhs/nhs.h
	IRoadsignDetection.h
	CRoadsignDetectionDLL.h
	cRoadSigns.h	
	roadsignDetectionHeaders.h	
	)

# include directories
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIR}) 


#the dll	
add_library(${LIB_NAME} SHARED 
            stdafx.cpp
            stdafx.h
            ${SOURCE}
            ${HEADER}
)


# linking libs
target_link_libraries(${LIB_NAME} ${ADTF_LIBS} ${OpenCV_LIBS})

# set folder (only needed and available for MSVC)
adtf_set_folder(${LIB_NAME} aadcbase/libs) 

# install commands
adtf_install_library(${LIB_NAME} ${AADC_INSTALL_DIR_LIBS})


