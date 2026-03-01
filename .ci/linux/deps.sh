#!/bin/sh -eux

# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

EXTRA_PACKAGES="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/get-debloated-pkgs.sh"

echo "Installing build dependencies..."
echo "---------------------------------------------------------------"
pacman -Syu --noconfirm --overwrite "*" \
	base-devel \
	cmake \
	curl \
	extra-cmake-modules \
	git \
	jdk8-openjdk \
	ninja \
	qt6-tools \
	qt6-charts \
	qt6-5compat \
	qt6-imageformats \
	scdoc \
	strace \
	unzip \
	xorg-server-xvfb \
	wget \
	zsync

if [ "$(uname -m)" = 'x86_64' ]; then
	pacman -Syu --noconfirm --overwrite "*" haskell-gnutls svt-av1
fi

echo "Installing debloated packages..."
echo "---------------------------------------------------------------"
wget --retry-connrefused --tries=30 "$EXTRA_PACKAGES"
chmod +x get-debloated-pkgs.sh
./get-debloated-pkgs.sh qt6-base-mini libxml2-mini icu-mini

echo "All done!"
echo "---------------------------------------------------------------"