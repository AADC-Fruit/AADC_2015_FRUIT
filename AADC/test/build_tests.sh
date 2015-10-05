#!/bin/sh -ex

# Parameter für Qt-Pfad

QT_DIR=/opt/arm-linux-gnueabihf/SDK/gcc-4.6/Qt/4.7.1
ADTF_DIR=/opt/arm-linux-gnueabihf/adtf/2.12.1B

## if commandline --qt-dir
##  QT_DIR=from commandline
## fi

## if commandline --adtf-dir
##  ADTF_DIR=from commandline
## fi

QMAKE_PATH=$QT_DIR/bin/qmake
ADTF_TOOLCHAIN_FILE=$ADTF_DIR/toolchainfiles/ARM_toolchain_build_on_target.cmake

if which cmake > /dev/null; then
    echo "cmake found"
else
    echo "cmake not found. Make sure it's in your PATH."
    exit 1
fi

if [ -f $QMAKE_PATH ]; then
    echo "qmake found."
else
    echo "qmake not found in $QMAKE_PATH. Check the path to Qt or set it as commandline parameter."
    exit 1

fi

if [ -d $ADTF_DIR ]; then
    echo "ADTF dir found."
else
    echo "ADTF dir not found in $ADTF_DIR. Check the path to ADTF or set it as commandline parameter."
    exit 1
fi

if [ -f $ADTF_TOOLCHAIN_FILE ]; then
    echo "ADTF toolchainfile found."
else
    echo "ADTF toolchain file not found in $ADTF_DIR. Check the path to ADTF or set it as commandline parameter."
    exit 1
fi

## if commandline cleanup before build
#     rm -rf _build_tests_release
## fi

if [ -d ./_build_tests_release ]; then
    echo "Build exists. Will using it."
else
    mkdir ./_build_tests_release
    echo "Creating build directory."
fi

echo "entering build directory"
cd ./_build_tests_release

echo "generate cmake config"
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$ADTF_TOOLCHAIN_FILE -DADTF_DIR=$ADTF_DIR -DQT_QMAKE_EXECUTABLE=$QMAKE_PATH -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../.. ..

echo "build ..."
cmake --build . --config RelWithDebInfo

cd ..

#if [ -d ./_build_tests_debug ]; then
#    echo "Build exists. Will using it."
#else
#    mkdir ./_build_tests_debug
#    echo "Creating build directory."
#fi
#
#echo "entering build directory"
#cd ./_build_tests_debug
#
#echo "generate cmake config"
#cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=$ADTF_TOOLCHAIN_FILE -DADTF_DIR=$ADTF_DIR -DQT_QMAKE_EXECUTABLE=$QMAKE_PATH -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../.. ..
#
#echo "build ..."
#cmake --build . --config Debug
#
#cd ..



