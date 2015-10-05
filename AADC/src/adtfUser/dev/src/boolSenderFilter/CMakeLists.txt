# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(boolSenderFilter
    stdafx.h
    stdafx.cpp
    
    bool_sender_filter.h
    bool_sender_filter.cpp
)

adtf_install_plugin(boolSenderFilter ${AADC_INSTALL_DIR})
