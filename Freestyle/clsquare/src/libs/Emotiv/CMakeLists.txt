CHECK_EXTERNAL ( Emotiv HIDAPI )
CHECK_EXTERNAL ( Emotiv MCrypt )
CHECK_EXTERNAL ( Emotiv boost_system )
CHECK_EXTERNAL ( Emotiv boost_thread )

IF ( NOT MISSING_DEPENDENCY )

  # external emokit

  INCLUDE ( ExternalProject )
  ExternalProject_Add ( libemokit
    GIT_REPOSITORY git://github.com/qdot/emokit.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    PREFIX "."
    UPDATE_COMMAND ""
  )
  LIST ( APPEND CLSQUARE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/libemokit/include )
  SET(CLSQUARE_INCLUDE_DIRS ${CLSQUARE_INCLUDE_DIRS} PARENT_SCOPE)

  # own library

  set ( lib_src
    ${CURRENT_LIST_DIR}/src/pipe_client.cpp
    ${CURRENT_LIST_DIR}/src/tcp_client.cpp
    ${CURRENT_LIST_DIR}/src/tcpsocket.cc
    ${CURRENT_LIST_DIR}/src/valueparser.cc
    ${CURRENT_LIST_DIR}/src/hid_client.cpp
  )
  set ( lib_interfaces
    ${CURRENT_LIST_DIR}/emotiv/emotiv_client.h
    ${CURRENT_LIST_DIR}/emotiv/pipe_client.h
    ${CURRENT_LIST_DIR}/emotiv/tcp_client.h
    ${CURRENT_LIST_DIR}/emotiv/tcpsocket.h
    ${CURRENT_LIST_DIR}/emotiv/hid_client.h
  )

  ADD_LIBRARY ( Emotiv STATIC ${lib_src} )
  ADD_DEPENDENCIES ( Emotiv libemokit )  
  target_link_libraries ( Emotiv emokit ${CLSQUARE_LIB_EXTERNAL} )
  include_directories ( ${CMAKE_CURRENT_BINARY_DIR}/src/libemokit/include )

  INSTALL ( FILES ${lib_interfaces} DESTINATION include/emotiv )
  INSTALL ( TARGETS Emotiv
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  # test server

  add_executable ( epocToPipe src/writeToPipe.cpp )
  target_link_libraries ( epocToPipe Emotiv emokit ${CLSQUARE_LIB_EXTERNAL} )
  INSTALL (
    TARGETS epocToPipe
    RUNTIME DESTINATION bin
  )

ENDIF()
