
include_directories(../../../../src/adtfBase/AADC_ADTF_BaseFilters/include)

adtf_add_test(tester_aadc_arduino_comm
    180
    "${CMAKE_INSTALL_PREFIX}/bin,${ADTF_DIR}/bin"
    "${CMAKE_INSTALL_PREFIX}/bin/debug,${ADTF_DIR}/bin/debug"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"

    stdafx.h
    stdafx.cpp
    main.cpp

    tester_aadc_arduino_comm.cpp
    tester_aadc_arduino_comm.h
)

adtf_set_folder(tester_aadc_arduino_comm test/funktion)
