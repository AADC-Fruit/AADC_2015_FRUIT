# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(crossroadScriptFilter
    stdafx.h
    stdafx.cpp
    
    crossroad_script_filter.h
    crossroad_script_filter.cpp
)

adtf_install_plugin(crossroadScriptFilter ${AADC_INSTALL_DIR})
