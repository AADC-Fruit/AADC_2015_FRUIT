# these will likely fail on non-hydro installations without environment variables set
find_path ( ROS_INCLUDE_DIR ros/ros.h HINTS /opt/local/include /usr/local/include /opt/ros/hydro/include )
find_library ( ROS_LIBRARY NAMES roscpp HINTS /opt/local/lib /usr/local/lib /opt/ros/hydro/lib )

# find additional libraries used
macro ( FIND_ROS_LIBRARY LIB )
  find_library ( ROS_LIBRARY_${LIB} NAMES ${LIB} HINTS /opt/local/lib /usr/local/lib )
  if ( ROS_LIBRARY_${LIB} AND ROS_LIBRARY )
    list ( APPEND ROS_LIBRARY ${ROS_LIBRARY_${LIB}} )
  else ()
    set ( ROS_LIBRARY "NOTFOUND" )
  endif ()
endmacro ()
FIND_ROS_LIBRARY(roscpp_serialization)
FIND_ROS_LIBRARY(rosconsole)
FIND_ROS_LIBRARY(rostime)

# output
if ( ROS_LIBRARY AND ROS_INCLUDE_DIR )
  set ( FOUND_ROS TRUE )
  set ( ROS_FOUND TRUE )
elseif ( NOT ROS_FIND_QUIETLY )
  message ( WARNING " could not find ROS (LIBRARY=${ROS_LIBRARY}, INCLUDE=${ROS_INCLUDE_DIR})" )
endif ()


set ( ROS_LIBRARIES ${ROS_LIBRARY} )
set ( ROS_INCLUDE_DIRS ${ROS_INCLUDE_DIR} )
mark_as_advanced ( ROS_INCLUDE_DIR ROS_LIBRARY )

# # find the catkin build library; we will need it for message generation
# if ( FOUND_ROS )
#   find_package ( catkin REQUIRED COMPONENTS roscpp rospy std_msgs message_generation )
# endif ()
