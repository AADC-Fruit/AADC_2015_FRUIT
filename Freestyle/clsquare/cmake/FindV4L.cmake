find_path ( V4L2_INCLUDE_DIR linux/videodev2.h HINTS /usr/include /opt/local/include )

# TODO on OSX we would need to find the libs as well, but not on Linux

if ( V4L2_INCLUDE_DIR )
  set ( V4L2_FOUND TRUE )
  if ( NOT V4L2_FIND_QUIETLY )
    message ( "-- Found Video4Linux2" )
  endif ()
else ()
  if ( NOT V4L2_FIND_QUIETLY )
    message ( "-- Could NOT find Video4Linux2. Dependant modules will not be compiled." )
  endif ()
endif ()
mark_as_advanced ( V4L2_INCLUDE_DIR )


