#!/bin/bash
mkdir builddir
cd builddir
cmake -DLauncher_LAYOUT=lin-system -DCMAKE_INSTALL_PREFIX=../polymc/usr ../../../
make -j$(nproc) install
cd ..
dpkg-deb --build polymc