# Thomas Lampe
# search for generic lib
find_path ( ${GENERIC_ID}_INCLUDE_DIR ${GENERIC_HEADER} HINTS /opt/local/include /usr/local/include )
find_library ( ${GENERIC_ID}_LIBRARY NAMES ${GENERIC_LIB}
             HINTS /opt/local/lib /usr/local/lib )
if ( ${GENERIC_ID}_LIBRARY AND ${GENERIC_ID}_INCLUDE_DIR )
  set ( FOUND_${GENERIC_ID} TRUE )
elseif ( NOT ${GENERIC_ID}_FIND_QUIETLY )
  message ( WARNING " could not find ${GENERIC_ID} (LIBRARY=${${GENERIC_ID}_LIBRARY}, INCLUDE=${${GENERIC_ID}_INCLUDE_DIR})" )
endif ()
set ( ${GENERIC_ID}_LIBRARIES ${${GENERIC_ID}_LIBRARY} )
set ( ${GENERIC_ID}_INCLUDE_DIRS ${${GENERIC_ID}_INCLUDE_DIR} )
mark_as_advanced ( ${GENERIC_ID}_INCLUDE_DIR ${GENERIC_ID}_LIBRARY )
