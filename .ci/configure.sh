#!/bin/sh -e

: "${INSTALL_DIR:=/usr}"
: "${QT_VERSION:=6}"
: "${BUILD_TYPE:=Release}"
: "${LEGACY_BUILD:=OFF}"

# macOS needs extra love :)
if uname -s | grep -i darwin >/dev/null 2>&1; then
	case "$QT_VERSION" in
		5)
			OSX_ARCH=x86_64
			set -- "$@" -DMACOSX_SPARKLE_UPDATE_PUBLIC_KEY="" -DMACOSX_SPARKLE_UPDATE_FEED_URL=""
			;;
		6) OSX_ARCH="x86_64;arm64" ;;
	esac

	set -- "$@" -DCMAKE_OSX_ARCHITECTURES="$OSX_ARCH"
fi

# non-linux uses $PWD/install
case "$(uname -s)" in
	Linux*) ;;
	*) INSTALL_DIR="$PWD/install"
esac

cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DENABLE_LTO=ON -DUSE_CCACHE=ON -DLauncher_QT_VERSION_MAJOR="$QT_VERSION" -G Ninja \
	-DLEGACY_BUILD="$LEGACY_BUILD" "$@"