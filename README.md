<p align="center">
<img src="./program_info/polymc-header-black.svg#gh-light-mode-only" alt="PolyMC logo" width="50%"/>
<img src="./program_info/polymc-header.svg#gh-dark-mode-only" alt="PolyMC logo" width="50%"/>
</p>

This is [**fork** of PolyMC](https://github.com/PolyMC/PolyMC) with offline patches for personal use.<br><br>
PolyMC is a custom launcher for Minecraft that focuses on predictability, long term stability and simplicity.

This is a **fork** of the MultiMC Launcher and not endorsed by MultiMC.
If you want to read about why this fork was created, check out [our FAQ page](https://polymc.org/wiki/overview/faq/).
<br>

# Installation

- Download last release [here](https://github.com/EvilToasterDBU/PolyMC-Offline/releases/latest)
- Last build status: https://github.com/EvilToasterDBU/PolyMC-Offline/actions

## Development Builds

There are per-commit development builds available [here](https://github.com/EvilToasterDBU/PolyMC-Offline/actions). These have debug information in the binaries, so their file sizes are relatively larger.
Portable builds are provided for AppImage on Linux, Windows, and macOS.

## Building

If you want to build PolyMC yourself, check [Build Instructions](https://polymc.org/wiki/development/build-instructions/) for build instructions.

## Translations

The translation effort for PolyMC is hosted on [Weblate](https://hosted.weblate.org/projects/polymc/polymc/) and information about translating PolyMC is available at <https://github.com/PolyMC/Translations>


## Forking/Redistributing/Custom builds policy

We don't care what you do with your fork/custom build as long as you follow the terms of the [license](LICENSE) (this is a legal responsibility), and if you made code changes rather than just packaging a custom build, please do the following as a basic courtesy:

- Make it clear that your fork is not PolyMC and is not endorsed by or affiliated with the PolyMC project (<https://polymc.org>).
- Go through [CMakeLists.txt](CMakeLists.txt) and change PolyMC's API keys to your own or set them to empty strings (`""`) to disable them (this way the program will still compile but the functionality requiring those keys will be disabled).

If you have any questions or want any clarification on the above conditions please make an issue and ask us.

Be aware that if you build this software without removing the provided API keys in [CMakeLists.txt](CMakeLists.txt) you are accepting the following terms and conditions:

- [Microsoft Identity Platform Terms of Use](https://docs.microsoft.com/en-us/legal/microsoft-identity-platform/terms-of-use)
- [CurseForge 3rd Party API Terms and Conditions](https://support.curseforge.com/en/support/solutions/articles/9000207405-curse-forge-3rd-party-api-terms-and-conditions)

If you do not agree with these terms and conditions, then remove the associated API keys from the [CMakeLists.txt](CMakeLists.txt) file by setting them to an empty string (`""`).

All launcher code is available under the GPL-3.0-only license.
  
The logo and related assets are under the CC BY-SA 4.0 license.

