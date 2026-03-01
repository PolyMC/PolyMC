#!/bin/sh -ex

ROOTDIR="$PWD"
INSTALL="$PWD/install"
ARTIFACTS_DIR="$PWD/artifacts"
BUILD="$PWD/build"

if [ "${LEGACY_BUILD:-false}" = 'true' ]; then
	_id="-Legacy"
fi

mkdir -p "$ARTIFACTS_DIR"
_artifact="$ARTIFACTS_DIR/PolyMC-macOS${_id}-${VERSION}.tar.gz"
EXE="$INSTALL/PolyMC.app/Contents/MacOS/polymc"

cmake --install "$BUILD" --prefix "$INSTALL"

cd "$INSTALL"
chmod +x "$EXE"
sudo codesign --sign - --deep --force --entitlements \
	"../program_info/App.entitlements" --options runtime "$EXE"

# TODO(crueter): Define artifact version, name, etc. separately.
tar -czf "$_artifact" ./*

cd "$ROOTDIR"

# Make sparkle sig
if [ -n "$SPARKLE_KEY" ]; then
	brew install openssl@3
	echo "$SPARKLE_KEY" > ed25519-priv.pem

	_openssl="$(brew --prefix openssl@3)"/bin/openssl
	signature=$("$_openssl" pkeyutl -sign -rawin \
		-in "$_artifact" -inkey ed25519-priv.pem \
		| openssl base64 | tr -d \\n)

	rm ed25519-priv.pem
	cat >> "$GITHUB_STEP_SUMMARY" <<-EOF
	### Artifact Information :information_source:
	- :memo: Sparkle Signature (ed25519): \`$signature\`
	EOF
else
	cat >> "$GITHUB_STEP_SUMMARY" <<-EOF
	### Artifact Information :information_source:
	- :warning: Sparkle Signature (ed25519): No private key available (likely a pull request or fork)
	EOF
fi
