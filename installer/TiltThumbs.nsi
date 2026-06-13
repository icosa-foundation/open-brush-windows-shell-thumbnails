; TiltThumbs Installer Script for NSIS
; This creates a user-friendly installer for the TiltThumbs shell extension

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "x64.nsh"

;--------------------------------
; General

Name "TiltThumbs"
!ifndef INSTALLER_OUTFILE
!define INSTALLER_OUTFILE "TiltThumbs-Installer.exe"
!endif
OutFile "${INSTALLER_OUTFILE}"
Unicode True

!ifndef INPUT_BIN_DIR
!define INPUT_BIN_DIR "..\build\bin\Release"
!endif

; Default installation folder
InstallDir "$PROGRAMFILES64\TiltThumbs"

; Request admin rights
RequestExecutionLevel admin

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Version Information

VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "TiltThumbs"
VIAddVersionKey "CompanyName" "Open Brush"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2024"
VIAddVersionKey "FileDescription" "Thumbnail provider for .tilt files"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "ProductVersion" "1.0.0.0"

;--------------------------------
; Installer Sections

Section "Install"
    ; Set output path to the installation directory
    SetOutPath "$INSTDIR"

    ; If upgrading, unregister the old DLL first and use /REBOOTOK for locked files
    ${If} ${FileExists} "$INSTDIR\TiltThumbs.dll"
        DetailPrint "Unregistering previous version..."
        ExecWait 'regsvr32.exe /u /s "$INSTDIR\TiltThumbs.dll"'
        Delete /REBOOTOK "$INSTDIR\TiltThumbs.dll"

        ; Icon refresh is handled by SHChangeNotify in DllRegisterServer
    ${EndIf}

    ; Copy DLL file (overwrites old version)
    File "${INPUT_BIN_DIR}\TiltThumbs.dll"
    File "${INPUT_BIN_DIR}\overlay-icon.png"

    ; Register the new DLL
    DetailPrint "Registering TiltThumbs..."
    ExecWait 'regsvr32.exe /s "$INSTDIR\TiltThumbs.dll"' $0

    ; Check if registration succeeded
    ${If} $0 != 0
        MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to register TiltThumbs.dll. Error code: $0"
        Abort
    ${EndIf}

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Create Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\TiltThumbs"
    CreateShortcut "$SMPROGRAMS\TiltThumbs\Uninstall TiltThumbs.lnk" "$INSTDIR\Uninstall.exe"

    ; Write registry keys for Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "DisplayName" "TiltThumbs"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "Publisher" "Open Brush"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs" "NoRepair" 1

    ; Check if file was locked and needs restart
    IfRebootFlag 0 NoRebootNeeded
        MessageBox MB_OK|MB_ICONINFORMATION "TiltThumbs has been installed!$\n$\nPlease restart your computer to complete the update."
        Goto Done

    NoRebootNeeded:
        MessageBox MB_OK|MB_ICONINFORMATION "TiltThumbs has been installed successfully!$\n$\nYour .tilt files will now show thumbnails in Windows Explorer."

    Done:
SectionEnd

;--------------------------------
; Uninstaller Section

Section "Uninstall"
    ; Unregister the DLL
    ExecWait 'regsvr32.exe /u /s "$INSTDIR\TiltThumbs.dll"'

    ; Delete files
    Delete "$INSTDIR\TiltThumbs.dll"
    Delete "$INSTDIR\overlay-icon.png"
    Delete "$INSTDIR\Uninstall.exe"

    ; Remove directory
    RMDir "$INSTDIR"

    ; Remove Start Menu shortcuts
    Delete "$SMPROGRAMS\TiltThumbs\Uninstall TiltThumbs.lnk"
    RMDir "$SMPROGRAMS\TiltThumbs"

    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TiltThumbs"

    ; Success message
    MessageBox MB_OK|MB_ICONINFORMATION "TiltThumbs has been uninstalled.$\n$\nYou may want to clear the thumbnail cache:$\n1. Run Disk Cleanup$\n2. Select 'Thumbnails'$\n3. Click OK"
SectionEnd
