# add the include directory of the filter
include_directories(../include)
# Specify the filter module target
adtf_add_filter(lightMachineFilter
    stdafx.cpp
    stdafx.h
    light_machine_filter.h
    light_machine_filter.cpp
)
# Specify where it should be installed to
adtf_install_plugin(lightMachineFilter ${AADC_INSTALL_DIR})
