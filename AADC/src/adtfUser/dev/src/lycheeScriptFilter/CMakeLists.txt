# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(lycheeScriptFilter
    stdafx.h
    stdafx.cpp
    
    lychee_script_filter.h
    lychee_script_filter.cpp
)

adtf_install_plugin(lycheeScriptFilter ${AADC_INSTALL_DIR})
