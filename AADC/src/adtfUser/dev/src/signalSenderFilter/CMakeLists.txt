# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(signalSenderFilter
    stdafx.h
    stdafx.cpp
    
    signal_sender_filter.h
    signal_sender_filter.cpp
)

adtf_install_plugin(signalSenderFilter ${AADC_INSTALL_DIR})
