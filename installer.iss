; CodeStudio Recorder Professional Installer Script
#define MyAppName "CodeStudio Recorder"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Khandev"
#define MyAppURL "https://github.com/khandev1211-cpu/CodeStudioRecorder"
#define MyAppExeName "codestudio_recorder.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
AppId={{C5D4B2A1-8E92-4F1A-9E3B-D4C5E6F7A8B9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=.\build\installer
OutputBaseFilename=CodeStudioRecorder_Setup
SetupIconFile=windows\runner\resources\app_icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Flutter Build Output
Source: "build\windows\x64\runner\Release\codestudio_recorder.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\windows\x64\runner\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs; Excludes: "codestudio_recorder.exe"
; Native DLL
Source: "build\windows\x64\native_engine\Release\codestudio_engine.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; FFmpeg DLLs
Source: "third_party\ffmpeg\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; AppUserModelID: "Khandev.CodeStudio.Recorder.1.0"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; AppUserModelID: "Khandev.CodeStudio.Recorder.1.0"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
