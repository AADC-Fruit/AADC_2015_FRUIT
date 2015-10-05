# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(laneFilter
    stdafx.h
    stdafx.cpp
    
    lane_filter.h
    lane_filter.cpp
    lane_preprocessor.h
    lane_preprocessor.cpp
    lane_detector.h
    lane_detector.cpp
    
    ../util/vector2.h
    ../util/vector2.cpp
    ../util/object.h
    ../util/object.cpp
    ../util/point_transformer.h
    ../util/point_transformer.cpp
)

target_link_libraries(laneFilter ${OpenCV_LIBS})
adtf_install_plugin(laneFilter ${AADC_INSTALL_DIR})
