#!/bin/sh -ex

ROOTDIR="$PWD"
INSTALL="$PWD/install"
INSTALL_PORTABLE="$PWD/install-portable"
ARTIFACTS_DIR="$PWD/artifacts"
BUILD="$PWD/build"

export PATH="/$MSYSTEM/bin:$PATH"

mkzip() {
	_dir="${1:?}"

	_prev="$PWD"

	# add additional identifier for portable, and legacy
	_id=""

	if [ "$PORTABLE" = 1 ]; then
		_id="${_id}-Portable"
	fi

	ZIP_NAME="PolyMC-Windows${_id}-${ARCH}-${VERSION}.zip"

	cd "$_dir"
	7z a -tzip "$ARTIFACTS_DIR/$ZIP_NAME" ./*
	cd "$_prev"
}

cmake --install "$BUILD" --prefix "$INSTALL"

mkzip "$INSTALL"

# portable package
cp -r "$INSTALL" "$INSTALL_PORTABLE"
cmake --install "$BUILD" --prefix "$INSTALL_PORTABLE" --component portable

PORTABLE=1 mkzip "$INSTALL_PORTABLE"

# setup package
: "${MSYS2_LOCATION:=C:/msys64}"
export PATH="$PATH:${MSYS2_LOCATION}/ucrt64/bin"

cd "$INSTALL"
makensis -NOCD "$BUILD/program_info/win_install.nsi"

cp "$ROOTDIR"/*.exe "$ARTIFACTS_DIR/PolyMC-Windows-Setup-${ARCH}-${VERSION}.exe"