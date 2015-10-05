REM set DT_TEST_PATTERN=tester_aadc_aktors
REM set DT_TEST_PATTERN=tester_aadc_sensors
REM set DT_TEST_PATTERN=tester_aadc_arduino_comm
REM set DT_TEST_PATTERN= 

call build_tests_win.bat

rem run debug tests
cd _build_tests

if "%DT_TEST_PATTERN%"=="" (
    ctest --interactive-debug-mode 0 --output-on-failure -C "Debug"
) else (
    ctest --interactive-debug-mode 0 --output-on-failure -C "Debug" -R "%DT_TEST_PATTERN%"
)
echo off
cd ..

REM #run release tests
REM cd _build_tests_release
REM ctest --interactive-debug-mode 0 --output-on-failure -C "Release"
REM rem -R "$DT_TEST_PATTERN" 
REM cd ..
