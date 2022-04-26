#!/usr/bin/env bash

# A script that fixes macOS Qt framework weirdness.
# See https://stackoverflow.com/questions/27952111/unable-to-sign-app-bundle-using-qt-frameworks-on-os-x-10-10/28097138#28097138

# Usage: ./macos_fix_qt_structure.sh path_to_app [major_qt_version]

# retrieve bundle name from first parameter
BUNDLE_NAME=$1

# retrieve Qt major version from second parameter (5 by default)
QT_MAJOR_VERSION=$2
QT_MAJOR_VERSION="${QT_MAJOR_VERSION:-5}"

# if can't change directory, then quit immediately
cd "${BUNDLE_NAME}/Contents/Frameworks/" || return 1

for CURRENT_FRAMEWORK in Qt*; do
    echo "Processing framework: ${CURRENT_FRAMEWORK}"

    CURRENT_FRAMEWORK_NAME="${CURRENT_FRAMEWORK%.*}"

    if [ ! -d "${CURRENT_FRAMEWORK}/Versions/${QT_MAJOR_VERSION}" ]; then
        echo "The Qt framework does not seem to contain version ${QT_MAJOR_VERSION}. Are you sure the version is correct?"
        exit 1
    fi

    mv "${CURRENT_FRAMEWORK}/Resources" "${CURRENT_FRAMEWORK}/Versions/${QT_MAJOR_VERSION}/Resources"

    ln -nfs "${QT_MAJOR_VERSION}"                        "${CURRENT_FRAMEWORK}/Versions/Current"
    ln -nfs "Versions/Current/${CURRENT_FRAMEWORK_NAME}" "${CURRENT_FRAMEWORK}/${CURRENT_FRAMEWORK_NAME}"
    ln -nfs "Versions/Current/Resources"                 "${CURRENT_FRAMEWORK}/Resources"
done
