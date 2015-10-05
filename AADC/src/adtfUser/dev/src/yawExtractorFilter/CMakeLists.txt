# add the include directory of the filter
include_directories(../include)
# Specify the filter module target
adtf_add_filter(yawExtractorFilter
    stdafx.cpp
    stdafx.h
    yaw_extractor_filter.h
    yaw_extractor_filter.cpp
)

# Specify where it should be installed to
adtf_install_plugin(yawExtractorFilter ${AADC_INSTALL_DIR})
