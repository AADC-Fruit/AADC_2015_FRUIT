# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(sensorPackageFilter
    stdafx.h
    stdafx.cpp
    
    sensor_package_filter.h
    sensor_package_filter.cpp
)

adtf_install_plugin(sensorPackageFilter ${AADC_INSTALL_DIR})
