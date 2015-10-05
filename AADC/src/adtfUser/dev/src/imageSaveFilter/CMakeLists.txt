# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(imageSaveFilter
    stdafx.h
    stdafx.cpp
    
    image_save_filter.h
    image_save_filter.cpp
)

adtf_install_plugin(imageSaveFilter ${AADC_INSTALL_DIR})
