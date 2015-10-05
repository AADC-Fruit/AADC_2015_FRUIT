if(NOT CMAKE_CL_64)
    set(FILTER_NAME aadc_QrReader)

    #set include directory
    include_directories(${ZBAR_INCLUDE_DIR})

    # add the source
    adtf_add_filter(${FILTER_NAME}
                    stdafx.cpp 	
                    stdafx.h 
                    cQrReader.cpp
                    cQrReader.h
    )
    # linking libs
    target_link_libraries(${FILTER_NAME} ${ZBAR_LIBS})

    # set folder (only needed and available for MSVC)
    adtf_set_folder(${FILTER_NAME} aadcbase/imageproc) 

    # Specify where it should be installed to
    adtf_install_plugin(${FILTER_NAME} ${AADC_INSTALL_DIR})
	
endif(NOT CMAKE_CL_64)