# Run this script from Windows, not MSYS2

import os
import subprocess

TEMPLATE = r"""; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "stargate"
!define PRODUCT_VERSION "{MAJOR_VERSION_NUM}.0"
!define PRODUCT_PUBLISHER "stargateaudio"

;Require admin rights on NT6+ (When UAC is turned on)
RequestExecutionLevel admin

SetCompressor /SOLID lzma

Name "{MAJOR_VERSION} {MINOR_VERSION}"
OutFile "{MAJOR_VERSION}-{MINOR_VERSION}-win64-installer.exe"
InstallDir "$PROGRAMFILES\stargateaudio@github\Stargate"

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING
  !define MUI_LICENSEPAGE_CHECKBOX

!include MUI2.nsh

;--------------------------------
;Modern UI Configuration
;Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gpl-3.0.txt"
;!insertmacro MUI_PAGE_COMPONENTS
;!insertmacro MUI_PAGE_DIRECTORY
;!insertmacro MUI_PAGE_STARTMENU pageid variable
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
;!insertmacro MUI_UNPAGE_LICENSE textfile
;!insertmacro MUI_UNPAGE_COMPONENTS
;!insertmacro MUI_UNPAGE_DIRECTORY
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"

Section
    UserInfo::getAccountType
    Pop $0

    # compare the result with the string "Admin" to see if the user is admin.
    # If match, jump 3 lines down.
    StrCmp $0 "Admin" +3

    # if there is not a match, print message and return
    MessageBox MB_OK "not admin: $0"
    Return
SectionEnd

Section "install"
    SetOutPath $INSTDIR
    writeUninstaller "$INSTDIR\uninstall.exe"
    File "dist\stargate.exe"
    File "files\share\pixmaps\{MAJOR_VERSION}.ico"
    createShortCut \
      "$SMPROGRAMS\Stargate DAW.lnk" \
      "$INSTDIR\{MAJOR_VERSION}.exe" \
      "" \
      "$INSTDIR\{MAJOR_VERSION}.ico"
SectionEnd

Section "uninstall"
  RMDir /r $INSTDIR
SectionEnd
"""

CWD = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(CWD, "meta.json")) as f:
    meta = fh.read()
MAJOR_VERSION = meta['version']['major']
MINOR_VERSION = meta['version']['minor']

NSIS = r"C:\Program Files (x86)\NSIS\Bin\makensis.exe"

template = TEMPLATE.format(
	MINOR_VERSION=MINOR_VERSION,
	MAJOR_VERSION=MAJOR_VERSION,
	MAJOR_VERSION_NUM=MAJOR_VERSION[-1],
)
template_name = "{0}.nsi".format(MAJOR_VERSION)
with open(template_name, "w") as f:
	f.write(template)
subprocess.call([NSIS, template_name])
