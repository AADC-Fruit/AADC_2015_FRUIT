#!/bin/bash

cd ${0%/*}

mkdir ../../eclipse_debug_build
mkdir ../../eclipse_release_build

cd ../../eclipse_debug_build
rm -rf *
cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -D CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -D CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES=FALSE ../clsquare


cd ../eclipse_release_build
rm -rf *
cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -D CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES=FALSE ../clsquare
