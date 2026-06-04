{ stdenv
, lib
, cmake
, ninja
, jdk8
, gamemode
, ghc_filesystem
, zlib
, file
, qtbase
, quazip
, extra-cmake-modules
, qtcharts
, qtwayland
  # flake
, self
, version
, libnbtplusplus
, tomlplusplus
, msaClientID ? ""
, gamemodeSupport ? stdenv.isLinux
, enableLTO ? false
}:

stdenv.mkDerivation {
  pname = "polymc-unwrapped";
  inherit version;

  src = lib.cleanSource self;

  nativeBuildInputs = [ cmake kdePackages.extra-cmake-modules ninja jdk8 ghc_filesystem file ];
  buildInputs = [ qtbase quazip zlib qtcharts ]
    ++ lib.optional (lib.versionAtLeast qtbase.version "6") qtwayland
    ++ lib.optional gamemodeSupport gamemode;

  postUnpack = ''
    # Copy libnbtplusplus
    rm -rf source/libraries/libnbtplusplus
    mkdir source/libraries/libnbtplusplus
    cp -a ${libnbtplusplus}/* source/libraries/libnbtplusplus
    chmod a+r+w source/libraries/libnbtplusplus/*
    # Copy tomlplusplus
    rm -rf source/libraries/tomlplusplus
    mkdir source/libraries/tomlplusplus
    cp -a ${tomlplusplus}/* source/libraries/tomlplusplus
    chmod a+r+w source/libraries/tomlplusplus/*
  '';

  dontWrapQtApps = true;

  cmakeFlags = [
    "-GNinja"
    "-DLauncher_QT_VERSION_MAJOR=${lib.versions.major qtbase.version}"
    "-DLauncher_BUILD_PLATFORM=nix"
  ]
  ++ lib.optionals enableLTO [ "-DENABLE_LTO=on" ]
  ++ lib.optionals (msaClientID != "") [ "-DLauncher_MSA_CLIENT_ID=${msaClientID}" ];

  meta = with lib; {
    homepage = "https://polymc.org/";
    downloadPage = "https://polymc.org/download/";
    changelog = "https://github.com/PolyMC/PolyMC/releases";
    description = "A free, open source launcher for Minecraft";
    longDescription = ''
      Allows you to have multiple, separate instances of Minecraft (each with
      their own mods, texture packs, saves, etc) and helps you manage them and
      their associated options with a simple interface.
    '';
    platforms = platforms.unix;
    license = licenses.gpl3Only;
    maintainers = with maintainers; [ starcraft66 kloenk ];
  };
}
