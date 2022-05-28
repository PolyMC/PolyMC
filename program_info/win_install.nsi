!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "MUI2.nsh"

Unicode true

Name "PolyMC"
InstallDir "$LOCALAPPDATA\Programs\PolyMC"
InstallDirRegKey HKCU "Software\PolyMC" "InstallDir"
RequestExecutionLevel user

;--------------------------------

; Pages

!insertmacro MUI_PAGE_WELCOME
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$InstDir\polymc.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Kurdish"
!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Uzbek"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "Afrikaans"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Esperanto"
!insertmacro MUI_LANGUAGE "Asturian"
!insertmacro MUI_LANGUAGE "Basque"
!insertmacro MUI_LANGUAGE "Pashto"
!insertmacro MUI_LANGUAGE "ScotsGaelic"
!insertmacro MUI_LANGUAGE "Georgian"
!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_LANGUAGE "Welsh"
!insertmacro MUI_LANGUAGE "Armenian"
!insertmacro MUI_LANGUAGE "Corsican"
!insertmacro MUI_LANGUAGE "Tatar"
!insertmacro MUI_LANGUAGE "Hindi"

;--------------------------------

; The stuff to install
Section "PolyMC"

  SectionIn RO

  nsExec::Exec /TIMEOUT=2000 'TaskKill /IM polymc.exe /F'

  SetOutPath $INSTDIR

  File "polymc.exe"
  File "qt.conf"
  File *.dll
  File /r "iconengines"
  File /r "imageformats"
  File /r "jars"
  File /r "platforms"
  File /r "styles"

  ; Write the installation path into the registry
  WriteRegStr HKCU Software\PolyMC "InstallDir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  ${GetParameters} $R0
  ${GetOptions} $R0 "/NoUninstaller" $R1
  ${If} ${Errors}
    !define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\PolyMC"
    WriteRegStr HKCU "${UNINST_KEY}" "DisplayName" "PolyMC"
    WriteRegStr HKCU "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\polymc.exe"
    WriteRegStr HKCU "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKCU "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
    WriteRegStr HKCU "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINST_KEY}" "Publisher" "PolyMC Contributors"
    WriteRegStr HKCU "${UNINST_KEY}" "ProductVersion" "${VERSION}"
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKCU "${UNINST_KEY}" "EstimatedSize" "$0"
    WriteRegDWORD HKCU "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINST_KEY}" "NoRepair" 1
    WriteUninstaller "$INSTDIR\uninstall.exe"
  ${EndIf}

SectionEnd

Section "Start Menu Shortcuts" SHORTCUTS

  CreateShortcut "$SMPROGRAMS\PolyMC.lnk" "$INSTDIR\polymc.exe" "" "$INSTDIR\polymc.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  nsExec::Exec /TIMEOUT=2000 'TaskKill /IM polymc.exe /F'

  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\PolyMC"
  DeleteRegKey HKCU SOFTWARE\PolyMC

  Delete $INSTDIR\polymc.exe
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\portable.txt

  Delete $INSTDIR\libbrotlicommon.dll
  Delete $INSTDIR\libbrotlidec.dll
  Delete $INSTDIR\libbz2-1.dll
  Delete $INSTDIR\libcrypto-1_1-x64.dll
  Delete $INSTDIR\libcrypto-1_1.dll
  Delete $INSTDIR\libdouble-conversion.dll
  Delete $INSTDIR\libfreetype-6.dll
  Delete $INSTDIR\libgcc_s_seh-1.dll
  Delete $INSTDIR\libgcc_s_dw2-1.dll
  Delete $INSTDIR\libglib-2.0-0.dll
  Delete $INSTDIR\libgraphite2.dll
  Delete $INSTDIR\libharfbuzz-0.dll
  Delete $INSTDIR\libiconv-2.dll
  Delete $INSTDIR\libicudt69.dll
  Delete $INSTDIR\libicuin69.dll
  Delete $INSTDIR\libicuuc69.dll
  Delete $INSTDIR\libintl-8.dll
  Delete $INSTDIR\libjasper-4.dll
  Delete $INSTDIR\libjpeg-8.dll
  Delete $INSTDIR\libmd4c.dll
  Delete $INSTDIR\libpcre-1.dll
  Delete $INSTDIR\libpcre2-16-0.dll
  Delete $INSTDIR\libpng16-16.dll
  Delete $INSTDIR\libssl-1_1-x64.dll
  Delete $INSTDIR\libssl-1_1.dll
  Delete $INSTDIR\libssp-0.dll
  Delete $INSTDIR\libstdc++-6.dll
  Delete $INSTDIR\libwebp-7.dll
  Delete $INSTDIR\libwebpdemux-2.dll
  Delete $INSTDIR\libwebpmux-3.dll
  Delete $INSTDIR\libwinpthread-1.dll
  Delete $INSTDIR\libzstd.dll
  Delete $INSTDIR\Qt5Core.dll
  Delete $INSTDIR\Qt5Gui.dll
  Delete $INSTDIR\Qt5Network.dll
  Delete $INSTDIR\Qt5Qml.dll
  Delete $INSTDIR\Qt5QmlModels.dll
  Delete $INSTDIR\Qt5Quick.dll
  Delete $INSTDIR\Qt5Svg.dll
  Delete $INSTDIR\Qt5WebSockets.dll
  Delete $INSTDIR\Qt5Widgets.dll
  Delete $INSTDIR\Qt5Xml.dll
  Delete $INSTDIR\zlib1.dll

  Delete $INSTDIR\qt.conf

  RMDir /r $INSTDIR\iconengines
  RMDir /r $INSTDIR\imageformats
  RMDir /r $INSTDIR\jars
  RMDir /r $INSTDIR\platforms
  RMDir /r $INSTDIR\styles

  Delete "$SMPROGRAMS\PolyMC.lnk"

  RMDir "$INSTDIR"

SectionEnd

;--------------------------------

; Extra command line parameters

Function .onInit
${GetParameters} $R0
${GetOptions} $R0 "/NoShortcuts" $R1
${IfNot} ${Errors}
  !insertmacro UnselectSection ${SHORTCUTS}
${EndIf}
FunctionEnd
