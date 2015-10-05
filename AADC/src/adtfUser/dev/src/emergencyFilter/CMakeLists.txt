# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(emergencyFilter
    stdafx.h
    stdafx.cpp
    
    emergency_filter.h
    emergency_filter.cpp
)

adtf_install_plugin(emergencyFilter ${AADC_INSTALL_DIR})
