{
  stdenv,
  lib,
  symlinkJoin,
  addDriverRunpath,
  polymc-unwrapped,
  wrapQtAppsHook,
  jdk8,
  jdk17,
  jdk21,
  jdk25,
  libX11,
  libXext,
  libXcursor,
  libXrandr,
  libXxf86vm,
  xrandr,
  gamemode,
  mangohud,
  mesa-demos,
  libpulseaudio,
  qtbase,
  libGL,
  vulkan-loader,
  glfw,
  openal,
  udev,
  wayland,
  qtwayland,
  msaClientID ? "",
  jdks ? [
    jdk25
    jdk21
    jdk17
    jdk8
  ],
  enableLTO ? false,
  gamemodeSupport ? stdenv.isLinux,
  additionalLibs ? [ ],
  additionalBins ? [ ],
  self,
  version,
}
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
  buildInputs = [
    qtbase
    qtwayland
  ];

  postBuild = ''
    wrapQtAppsHook
  '';

  qtWrapperArgs =
    let
      runtimeLibs = [
        libX11
        libXext
        libXcursor
        libXrandr
        libXxf86vm
      ]
      ++
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
        xrandr
        mesa-demos # For glxinfo
      ]
      ++ additionalBins;
    in
    [
      "--prefix POLYMC_JAVA_PATHS : ${lib.makeSearchPath "bin/java" jdks}"
      "--set LD_LIBRARY_PATH ${addDriverRunpath.driverLink}/lib:${lib.makeLibraryPath runtimeLibs}"
      "--prefix PATH : ${lib.makeBinPath runtimeBins}"
    ];

  inherit (polymcInner) meta;
}
