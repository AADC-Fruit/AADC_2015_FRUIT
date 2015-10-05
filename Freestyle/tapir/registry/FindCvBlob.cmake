find_path( CvBlob_INCLUDE_DIR cvblob.h HINTS /usr/include /usr/local/include /opt/local/include )
find_library ( CvBlob_LIBRARY NAMES cvblob HINTS /usr/lib /usr/local/lib /opt/local/lib )

set ( CvBlob_LIBRARIES ${CvBlob_LIBRARY} )
set ( CvBlob_INCLUDE_DIRS ${CvBlob_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( CvBlob DEFAULT_MSG CvBlob_LIBRARY CvBlob_INCLUDE_DIR )

mark_as_advanced ( CvBlob_INCLUDE_DIR CvBlob_LIBRARY )

