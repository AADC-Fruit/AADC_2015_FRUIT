
rm -rf build
mkdir build
cd build
#cmake .. -DCMAKE_INSTALL_PREFIX="../" -DCMAKE_BUILD_TYPE=DEBUG
cmake ..
make -j2
make install
