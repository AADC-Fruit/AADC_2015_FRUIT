# add the include directory of the filter
include_directories(../include)
# Specify the filter module target
adtf_add_filter(speedCalculatorFilter
    stdafx.cpp
    stdafx.h
    speed_calculator_filter.h
    speed_calculator_filter.cpp
    cSlidingWindow.cpp
    cSlidingWindow.h
)
# Specify where it should be installed to
adtf_install_plugin(speedCalculatorFilter ${AADC_INSTALL_DIR})
