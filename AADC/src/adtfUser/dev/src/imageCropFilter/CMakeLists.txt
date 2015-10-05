# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(imageCropFilter
    stdafx.h
    stdafx.cpp

    image_crop_filter.h
    image_crop_filter.cpp
)

target_link_libraries(imageCropFilter ${OpenCV_LIBS})
adtf_install_plugin(imageCropFilter ${AADC_INSTALL_DIR})
