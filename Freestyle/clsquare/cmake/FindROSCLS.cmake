# ROSCLS is not a standard ROS node, so there's no telling where it is installed.
# With any luck, it will be in the 

# TODO won't find file if different catkin workspace, but there's no environment variable for it either
find_path ( ROSCLS_INCLUDE_DIR roscls/plant_init.h
  PATHS
  /opt/local/include
  /usr/local/include
  ~/catkin/install/include
  ~/catkin_ws/install/include
  ${ROSCLS_DIR}
)

# the headers are all we need
if ( ROSCLS_INCLUDE_DIR )
  set ( ROSCLS_FOUND TRUE )
  set ( ROSCLS_INCLUDE_DIRS ${ROSCLS_INCLUDE_DIR} )
endif ()

mark_as_advanced ( ROSCLS_INCLUDE_DIR )
