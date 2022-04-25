#!/usr/bin/env bash

# A script that fixes macOS Qt framework weirdness.
# See https://stackoverflow.com/questions/27952111/unable-to-sign-app-bundle-using-qt-frameworks-on-os-x-10-10/28097138#28097138

# retrieve bundle name from first parameter
BUNDLE_NAME=$1

# if can't change directory, then quit immediately
cd "${BUNDLE_NAME}/Contents/Frameworks/" || return 1

for CURRENT_FRAMEWORK in Qt*; do
    echo "Processing framework: ${CURRENT_FRAMEWORK}"

    CURRENT_FRAMEWORK_NAME="${CURRENT_FRAMEWORK%.*}"

    mkdir -p "${CURRENT_FRAMEWORK}/Versions/5"

    mv "${CURRENT_FRAMEWORK}/Resources" "${CURRENT_FRAMEWORK}/Versions/5/Resources"

    ln -nfs "5"                                          "${CURRENT_FRAMEWORK}/Versions/Current"
    ln -nfs "Versions/Current/${CURRENT_FRAMEWORK_NAME}" "${CURRENT_FRAMEWORK}/${CURRENT_FRAMEWORK_NAME}"
    ln -nfs "Versions/Current/Resources"                 "${CURRENT_FRAMEWORK}/Resources"
done
