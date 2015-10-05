# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(moose
    stdafx.h
    stdafx.cpp

    moose.h
    moose.cpp
)

adtf_install_plugin(moose ${AADC_INSTALL_DIR})
