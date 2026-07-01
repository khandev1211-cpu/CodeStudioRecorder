; CodeStudio Recorder Installer Script (Inno Setup)

[Setup]
AppName=CodeStudio Recorder
AppVersion=1.0.0
DefaultDirName={pf}\CodeStudioRecorder
DefaultGroupName=CodeStudio Recorder
OutputDir=.\build\installer
OutputBaseFilename=CodeStudioRecorder_Setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
SetupIconFile=windows\runner\resources\app_icon.ico

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Flutter Build Output
Source: "build\windows\x64\runner\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
; Native DLL
Source: "build\windows\x64\native_engine\Release\codestudio_engine.dll"; DestDir: "{app}"; Flags: ignoreversion
; FFmpeg DLLs (Manual addition)
Source: "third_party\ffmpeg\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\CodeStudio Recorder"; Filename: "{app}\CodeStudioRecorder.exe"
Name: "{commondesktop}\CodeStudio Recorder"; Filename: "{app}\CodeStudioRecorder.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\CodeStudioRecorder.exe"; Description: "{cm:LaunchProgram,CodeStudio Recorder}"; Flags: nowait postinstall skipifsilent
