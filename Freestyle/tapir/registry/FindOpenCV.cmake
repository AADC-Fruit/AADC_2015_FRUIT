find_path( OpenCV_INCLUDE_DIR opencv2/opencv.hpp HINTS /usr/include /usr/local/include /opt/local/include )
find_library ( OpenCV_LIBRARY NAMES cv_core HINTS /usr/lib /usr/local/lib /opt/local/lib )
set ( OpenCV_LIBRARIES ${OpenCV_LIBRARY} )
set ( OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( OpenCV DEFAULT_MSG OpenCV_LIBRARY OpenCV_INCLUDE_DIR )
mark_as_advanced ( OpenCV_INCLUDE_DIR OpenCV_LIBRARY )
