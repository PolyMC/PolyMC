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
	if [ "${LEGACY_BUILD:-false}" = 'true' ]; then
		_id="-Legacy"
	fi

	if [ "$PORTABLE" = 1 ]; then
		_id="${_id}-Portable"
	fi

	ZIP_NAME="PolyMC-Windows${_id}-${ARCH}-${VERSION}.zip"

	cd "$_dir"
	7z a -tzip "$ARTIFACTS_DIR/$ZIP_NAME" ./*
	cd "$_prev"
}

cmake --install "$BUILD" --prefix "$INSTALL"
[ "$QT_VERSION" = 6 ] || cp /mingw64/bin/lib*-3-x64.dll "$INSTALL"

mkzip "$INSTALL"

# portable package
cp -r "$INSTALL" "$INSTALL_PORTABLE"
cmake --install "$BUILD" --prefix "$INSTALL_PORTABLE" --component portable

PORTABLE=1 mkzip "$INSTALL_PORTABLE"

# setup package
cd "$INSTALL"
makensis -NOCD "$BUILD/program_info/win_install.nsi"

if [ "${LEGACY_BUILD:-false}" = 'true' ]; then
	_id="-Legacy"
else
	_id=""
fi

cp "$ROOTDIR"/*.exe "$ARTIFACTS_DIR/PolyMC-Windows-Setup${_id}-${ARCH}-${VERSION}.exe"