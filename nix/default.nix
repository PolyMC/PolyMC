{ stdenv
, lib
, symlinkJoin
, addDriverRunpath
, polymc-unwrapped
, wrapQtAppsHook
, jdk8
, jdk17
, jdk21
, xorg
, gamemode
, mesa-demos
, libpulseaudio
, qtbase
, libGL
, vulkan-loader
, glfw
, openal
, udev
, wayland
, qtwayland
, msaClientID ? ""
, jdks ? [ jdk21 jdk17 jdk8 ]
, enableLTO ? false
, gamemodeSupport ? stdenv.isLinux
, additionalLibs ? [ ]
, additionalBins ? [ ]
, self
, version
  # flake
}:

let
  polymcInner = polymc-unwrapped.override { inherit msaClientID enableLTO gamemodeSupport; };
in

symlinkJoin {
  name = "polymc";
  inherit version;

  paths = [ polymcInner ];

  nativeBuildInputs = [ wrapQtAppsHook ];
  buildInputs = [ qtbase qtwayland ];

  postBuild = ''
    wrapQtAppsHook
  '';

  qtWrapperArgs =
    let
      runtimeLibs = (with xorg; [
        libX11
        libXext
        libXcursor
        libXrandr
        libXxf86vm
      ]) ++
      # lwjgl
      [
        libpulseaudio
        libGL
        glfw
        openal
        stdenv.cc.cc.lib
        udev # OSHI
        wayland
        vulkan-loader # VulkanMod's lwjgl
      ]
      ++ lib.optional gamemodeSupport gamemode.lib
      ++ additionalLibs;

      runtimeBins = [
        # Required by old LWJGL versions
        xorg.xrandr
        mesa-demos # For glxinfo
      ] ++ additionalBins;
    in
    [
      "--prefix POLYMC_JAVA_PATHS : ${lib.makeSearchPath "bin/java" jdks}"
      "--set LD_LIBRARY_PATH ${addDriverRunpath.driverLink}/lib:${lib.makeLibraryPath runtimeLibs}"
      "--prefix PATH : ${lib.makeBinPath runtimeBins}"
    ];

  inherit (polymcInner) meta;
}
