#!/bin/bash -ex

ok="true"
####################################################
####################################################
# function declarations

# function "do_tests" will perform the tests
# Argument 1: Test configuration type (Debug, RelWithDebInfo)
# Argument 2: TEST_PATTERN
do_tests(){
    DT_CONFIG_TYPE=$1
    DT_TEST_PATTERN=$2
    
    if [ -n "$DT_TEST_PATTERN" ]; then
        if ctest --interactive-debug-mode 0 --output-on-failure -C "$DT_CONFIG_TYPE" -R "$DT_TEST_PATTERN"; then
            echo "All tests successfull."
        else
            echo "Error in some tests. Check the test result files."
            ok="false"
        fi
    else
        if ctest --interactive-debug-mode 0 --output-on-failure -C "$DT_CONFIG_TYPE"; then
            echo "All tests successfull."
        else
            echo "Error in some tests. Check the test result files."
            ok="false"
        fi
    fi
}



# end if function declaration
#####################################################
#####################################################

./build_tests.sh
    
##run debug tests
#cd _build_tests_debug
#do_tests "Debug" "$TEST_PATTERN"
#cd ..

#run release tests
cd _build_tests_release
do_tests "RelWithDebInfo" "$TEST_PATTERN"
cd ..
    

if [ "$ok" != "true" ]; then
    exit 1
fi
