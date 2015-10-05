# add the include directory of the filter
include_directories(../include)
# Specify the filter module target
adtf_add_filter(wheelLockFilter
    stdafx.cpp
    stdafx.h
    wheel_lock_filter.h
    wheel_lock_filter.cpp
)
# Specify where it should be installed to
adtf_install_plugin(wheelLockFilter ${AADC_INSTALL_DIR})
