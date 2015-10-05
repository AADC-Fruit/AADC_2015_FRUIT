# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(printFilter
    stdafx.h
    stdafx.cpp

    printFilter.h
    printFilter.cpp
)

adtf_install_plugin(printFilter ${AADC_INSTALL_DIR})
