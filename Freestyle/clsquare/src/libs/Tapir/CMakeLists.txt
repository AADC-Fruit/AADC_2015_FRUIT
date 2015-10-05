CHECK_EXTERNAL ( Tapir pthread )

IF ( NOT MISSING_DEPENDENCY )

  SET ( lib_src
    src/carreracam.cpp
    src/tapir.cpp
    src/Socket.cc
    src/valueparser.cc
  )

  SET ( lib_interfaces
    tapir/carreracam.h
    tapir/tapir.h
  )

  CHECK_INTERNAL ( KatanaSimCam FLI OPTIONAL )
  IF ( WITHLIB_FLI )
    LIST ( APPEND lib_interfaces tapir/katanasimcam.h )
    LIST ( APPEND lib_src src/katanasimcam.cpp )
  ENDIF ()

  ADD_LIBRARY ( Tapir STATIC ${lib_src} )

  INSTALL ( FILES ${lib_interfaces} DESTINATION include/tapir )
  INSTALL ( TARGETS Tapir
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
  )
  target_link_libraries ( Tapir )

ENDIF ()
