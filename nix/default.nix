{ stdenv
, lib
, fetchFromGitHub
, cmake
, ninja
, jdk8
, jdk
, zlib
, file
, wrapQtAppsHook
, xorg
, libpulseaudio
, mangohud
, qtbase
, quazip
, libGL
, msaClientID ? ""
, extraJDKs ? [ ]
, extra-cmake-modules

  # flake
, self
, version
, libnbtplusplus
, enableLTO ? false
}:

let
  # Libraries required to run Minecraft
  libpath = with xorg; lib.makeLibraryPath [
    libX11
    libXext
    libXcursor
    libXrandr
    libXxf86vm
    libpulseaudio
    libGL
    stdenv.cc.cc.lib
  ];

  # This variable will be passed to Minecraft by PolyMC
  # MangoHUD needs to be handled specially because it
  # puts library files in lib/mangohud/ rather than lib/
  gameLibraryPath = libpath + ":${mangohud}/lib/mangohud:/run/opengl-driver/lib";

  javaPaths = lib.makeSearchPath "bin/java" ([ jdk jdk8 ] ++ extraJDKs);
in

stdenv.mkDerivation rec {
  pname = "polymc";
  inherit version;

  src = lib.cleanSource self;

  nativeBuildInputs = [ cmake extra-cmake-modules ninja jdk file wrapQtAppsHook ];
  buildInputs = [ qtbase quazip zlib ];

  dontWrapQtApps = true;

  postUnpack = ''
    # Copy libnbtplusplus
    rm -rf source/libraries/libnbtplusplus
    mkdir source/libraries/libnbtplusplus
    cp -a ${libnbtplusplus}/* source/libraries/libnbtplusplus
    chmod a+r+w source/libraries/libnbtplusplus/*
  '';

  cmakeFlags = [
    "-GNinja"
    "-DLauncher_QT_VERSION_MAJOR=${lib.versions.major qtbase.version}"
  ] ++ lib.optionals enableLTO [ "-DENABLE_LTO=on" ]
    ++ lib.optionals (msaClientID != "") [ "-DLauncher_MSA_CLIENT_ID=${msaClientID}" ];

  postInstall = ''
    # xorg.xrandr needed for LWJGL [2.9.2, 3) https://github.com/LWJGL/lwjgl/issues/128
    wrapQtApp $out/bin/polymc \
      --set GAME_LIBRARY_PATH ${gameLibraryPath} \
      --prefix POLYMC_JAVA_PATHS : ${javaPaths} \
      --prefix PATH : ${lib.makeBinPath [ xorg.xrandr ]}
  '';

  meta = with lib; {
    homepage = "https://polymc.org/";
    description = "A free, open source launcher for Minecraft";
    longDescription = ''
      Allows you to have multiple, separate instances of Minecraft (each with
      their own mods, texture packs, saves, etc) and helps you manage them and
      their associated options with a simple interface.
    '';
    platforms = platforms.unix;
    license = licenses.gpl3Plus;
    maintainers = with maintainers; [ starcraft66 kloenk ];
  };
}
