# - Try to find libfltk
# Once done this will define
#  FLTK_FOUND - System has libfltk
#  FLTK_INCLUDE_DIRS - The libfltk include directories
#  FLTK_LIBRARIES - The libraries needed to use libfltk

find_path( FLTK_INCLUDE_DIR Fl.H HINTS /usr/include/FL /opt/local/include/FL )
find_library ( FLTK_LIBRARY NAMES fltk HINTS /usr/local/lib /opt/local/lib )

set ( FLTK_LIBRARIES ${FLTK_LIBRARY} )
set ( FLTK_INCLUDE_DIRS ${FLTK_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set FLTK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(fltk  DEFAULT_MSG FLTK_LIBRARY FLTK_INCLUDE_DIR)

mark_as_advanced ( FLTK_INCLUDE_DIR FLTK_LIBRARY )

