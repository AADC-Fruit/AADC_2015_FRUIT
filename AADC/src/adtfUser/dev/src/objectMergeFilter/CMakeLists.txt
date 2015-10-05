# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(objectMergeFilter
    stdafx.h
    stdafx.cpp
    
    object_merge_filter.h
    object_merge_filter.cpp
    ../util/object.h
    ../util/object.cpp
)

target_link_libraries(objectMergeFilter ${OpenCV_LIBS})
adtf_install_plugin(objectMergeFilter ${AADC_INSTALL_DIR})
