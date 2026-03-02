#!/bin/sh -e

mkdir -p dist

find . -type f \( \
    -name "PolyMC*.tar.gz" -o \
    -name "PolyMC*.exe" -o \
    -name "PolyMC*.zip" -o \
    -name "PolyMC*.AppImage*" \
\) -not -path "./dist/*" -exec cp {} dist/ \;

# mk source tarball
mv PolyMC-source PolyMC-"${VERSION}"
tar czf "dist/PolyMC-$VERSION.tar.gz" "PolyMC-${VERSION}"

echo "-- artifacts installed in $PWD/dist"

ls -lh dist