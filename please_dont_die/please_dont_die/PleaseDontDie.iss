; PLEASE, DON'T DIE v0.1 - Inno Setup script
; Compile with Inno Setup (F9). Run "build_mingw.bat dist" first so that
; dist\PleaseDontDie contains the .exe, data\ and audio\.
; All paths are relative to the folder where this .iss file lives.

#define MyAppName "PLEASE, DON'T DIE"
#define MyAppVersion "0.1"
#define MyAppPublisher "DXGPANIC"
#define MyAppExe "please_dont_die.exe"
#define SourceDir "dist\PleaseDontDie"

[Setup]
AppId={{8F3B2C1E-6A4D-4E7B-9C2F-1A2B3C4D5E6F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\PleaseDontDie
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=installer
OutputBaseFilename=PleaseDontDie-v{#MyAppVersion}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
UninstallDisplayIcon={app}\{#MyAppExe}

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "Crear acceso directo en el escritorio"; GroupDescription: "Accesos directos:"; Flags: unchecked

[Dirs]
Name: "{app}\saves"

[Files]
Source: "{#SourceDir}\{#MyAppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\audio\*"; DestDir: "{app}\audio"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"; WorkingDir: "{app}"
Name: "{group}\Desinstalar {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExe}"; Description: "Ejecutar {#MyAppName}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent
