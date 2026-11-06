#!/bin/sh -e

: "${INSTALL_DIR:=/usr}"
: "${BUILD_TYPE:=Release}"

# macOS needs extra love :)
if uname -s | grep -i darwin >/dev/null 2>&1; then
	OSX_ARCH="x86_64;arm64"
	set -- "$@" -DCMAKE_OSX_ARCHITECTURES="$OSX_ARCH"
fi

# non-linux uses $PWD/install
case "$(uname -s)" in
	Linux*) ;;
	*) INSTALL_DIR="$PWD/install"
esac

cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DENABLE_LTO=ON -DUSE_CCACHE=ON -G Ninja "$@"