# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(motorFilter
    stdafx.h
    stdafx.cpp

    motor_filter.h
    motor_filter.cpp
)

adtf_install_plugin(motorFilter ${AADC_INSTALL_DIR})
