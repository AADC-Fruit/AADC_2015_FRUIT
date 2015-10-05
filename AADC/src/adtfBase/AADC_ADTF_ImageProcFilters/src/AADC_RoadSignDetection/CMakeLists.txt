set(FILTER_NAME aadc_roadSignDetection)

#set directories
set (ROADSIGN_LINK_DIR ${CMAKE_INSTALL_PREFIX}/${AADC_INSTALL_DIR_LIBS})
link_directories(${ROADSIGN_LINK_DIR})
include_directories(${AADC_ROADSIGN_INCLUDE_DIR})

# add the source
adtf_add_filter(${FILTER_NAME}
                stdafx.cpp  
                stdafx.h
                RoadSignDetectPlugin.cpp
                RoadSignDetectHeaders.h
                RoadSignDetectFilter.h
                RoadSignDetectFilter.cpp
)

# linking libs
target_link_libraries(${FILTER_NAME} trafficsignrecognition)

# set folder (only needed and available for MSVC)
adtf_set_folder(${FILTER_NAME} aadcbase/imageproc) 

# Specify where it should be installed to
adtf_install_plugin(${FILTER_NAME} ${AADC_INSTALL_DIR})
	
