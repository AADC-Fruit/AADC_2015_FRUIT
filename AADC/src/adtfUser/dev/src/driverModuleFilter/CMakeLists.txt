# add the include directory of the filter
include_directories(../include)
# Specify the filter module target
adtf_add_filter(driverModuleFilter
    stdafx.cpp
    stdafx.h
    driver_module_filter.h
    driver_module_filter.cpp
    maneuverlist.h
)
# Specify where it should be installed to
adtf_install_plugin(driverModuleFilter ${AADC_INSTALL_DIR})
