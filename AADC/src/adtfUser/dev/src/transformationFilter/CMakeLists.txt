# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(transformationFilter
    stdafx.h
    stdafx.cpp

    transformation_filter.h
    transformation_filter.cpp
    map_transformation.h
    map_transformation.cpp
    
    ../util/vector2.h
    ../util/vector2.cpp
)

adtf_install_plugin(transformationFilter ${AADC_INSTALL_DIR})
