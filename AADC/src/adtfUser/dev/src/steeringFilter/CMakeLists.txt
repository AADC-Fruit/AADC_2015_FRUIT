# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(steeringFilter
    stdafx.h
    stdafx.cpp

    steering_filter.h
    steering_filter.cpp
)

adtf_install_plugin(steeringFilter ${AADC_INSTALL_DIR})
