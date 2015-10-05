# add the include directory of the filter
include_directories(../include)
# declare our filter
adtf_add_filter(mapFilter
    stdafx.h
    stdafx.cpp
    
    ../util/map/sensor_map.h
    ../util/map/sensor_map.cpp
    ../util/map/object_map.h
    ../util/map/object_map.cpp
    ../util/map/lane_map.h
    ../util/map/lane_map.cpp
    ../util/map/cell.h
    ../util/map/cell.cpp
    ../util/map/map.h
    ../util/map/map.cpp

    ../util/sensor/sensor_model.h
    ../util/sensor/sensor_model.cpp
    ../util/sensor/sensor_ir_long.h
    ../util/sensor/sensor_ir_long.cpp
    ../util/sensor/sensor_ir_short.h
    ../util/sensor/sensor_ir_short.cpp
    ../util/sensor/sensor_beam.h
    ../util/sensor/sensor_beam.cpp
    ../util/sensor/sensor_us.h
    ../util/sensor/sensor_us.cpp
    
    ../util/vector2.h
    ../util/vector2.cpp
    ../util/object.h
    ../util/object.cpp
    
    ../util/gridSearch/CompareSearchNode.h
    ../util/gridSearch/GridSearch.h
    ../util/gridSearch/GridSearch.cpp
    ../util/gridSearch/SearchMap.h   
    ../util/gridSearch/SearchMap.cpp
    ../util/gridSearch/SearchNode.h
    ../util/gridSearch/SearchNode.cpp
    ../util/gridSearch/Action.h
    ../util/gridSearch/Action.cpp
    ../util/gridSearch/ActionSet.h
    ../util/gridSearch/ActionSet.cpp
    
    ../util/gridSearch/aStarSearch/a_star_action_set.h
    ../util/gridSearch/aStarSearch/a_star_action_set.cpp
    ../util/gridSearch/aStarSearch/a_star_search_node.h
    ../util/gridSearch/aStarSearch/a_star_search_node.cpp
    ../util/gridSearch/aStarSearch/a_star.h
    ../util/gridSearch/aStarSearch/a_star.cpp
    
    ../util/control/steering_control.h
    ../util/control/steering_control.cpp
    
    ../util/point_transformer.h
    ../util/point_transformer.cpp
    
    map_filter.h
    map_filter.cpp
)

target_link_libraries(mapFilter ${OpenCV_LIBS})
adtf_install_plugin(mapFilter ${AADC_INSTALL_DIR})
