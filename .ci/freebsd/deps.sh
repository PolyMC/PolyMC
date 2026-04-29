#!/bin/sh -e

pkg update
pkg install -y \
    ninja \
    kf6-extra-cmake-modules \
    scdoc \
    qt6-base \
    qt6-charts \
    qt6-5compat \
    qt6-imageformats \
	cmake \
	openjdk8