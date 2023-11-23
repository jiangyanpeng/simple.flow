#!/usr/bin/bash
rm -rf build && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=simple.pipe/ -DBUILD_USE_AVX=ON -DBUILD_LOG=ON ..
make -j4 && make install
