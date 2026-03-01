#!/bin/sh -e

ROOTDIR="$PWD"
BUILDDIR="${BUILDDIR:-$ROOTDIR/build}"
ARTIFACTS_DIR="$ROOTDIR/artifacts"
INSTALL="$PWD/install"

SHARUN="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"

cmake --install "$BUILDDIR" --prefix "${INSTALL}/usr"

# variables to be used on quick-sharun and uruntime2appimage
export ICON="$ROOTDIR/program_info/org.polymc.PolyMC.svg"
export DESKTOP="$BUILDDIR/program_info/org.polymc.PolyMC.desktop"
export OPTIMIZE_LAUNCH=1
export DEPLOY_OPENGL=0
export DEPLOY_VULKAN=0
export ADD_HOOKS=""
export OUTPATH="$ARTIFACTS_DIR"
export OUTNAME="PolyMC-Linux-$ARCH-$VERSION.AppImage"
UPINFO="gh-releases-zsync|PolyMC|PolyMC|latest|PolyMC-Linux-${ARCH}-*.AppImage.zsync"

export UPINFO

# deploy
curl -L --retry 30 "$SHARUN" -o quick-sharun
chmod a+x quick-sharun
./quick-sharun "${INSTALL}/usr/bin/polymc" "${INSTALL}/usr/share/"

# MAKE APPIMAGE WITH URUNTIME
echo "-- Generating AppImage..."
./quick-sharun --make-appimage

echo "Linux package created: $OUTPATH/$OUTNAME"