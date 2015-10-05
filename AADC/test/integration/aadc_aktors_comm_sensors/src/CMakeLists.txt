include_directories(../../../../src/adtfBase/AADC_ADTF_BaseFilters/include)

adtf_add_test(tester_aadc_aktors_comm_sensors
    180
    "${CMAKE_INSTALL_PREFIX}/bin,${ADTF_DIR}/bin"
    "${CMAKE_INSTALL_PREFIX}/bin/debug,${ADTF_DIR}/bin/debug"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"

    stdafx.h
    stdafx.cpp
    main.cpp

    tester_aadc_aktors_comm_sensors.cpp
    tester_aadc_aktors_comm_sensors.h
)

adtf_set_folder(tester_aadc_aktors_comm_sensors test/integration)
