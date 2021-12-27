#!/bin/bash
mkdir builddir
cd builddir
cmake -DLauncher_LAYOUT=lin-system -DCMAKE_INSTALL_PREFIX=../polymc/usr ../../../
make -j$(nproc) install
cd ..
VER=$(git describe --tags | sed 's/-.*//')
sed -i "2s/.*/Version: $VER/" polymc/DEBIAN/control
dpkg-deb --build polymc
sed -i "2s/.*/Version: Set to latest git tag by makedeb.sh at build time/" polymc/DEBIAN/control