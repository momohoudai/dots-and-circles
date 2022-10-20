
[Setup]
AppId={{3914730F-73C7-4831-B62D-432927778943}
AppName=Dots and Circles
AppVerName=Dots and Circles 1.01
AppVersion=1.01


DefaultDirName={autopf}\momodevelop\Dots And Circles
DefaultGroupName=momodevelop\Dots And Circles




; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64

; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

; Output options
OutputBaseFilename=dnc_win_x64_setup
OutputDir=output

; Compression
Compression=lzma2
SolidCompression=yes

UninstallDisplayIcon={app}\setup.ico



[Code]
function GetUninstallRegPath: string;
begin
  Result := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\' + '{#SetupSetting("AppId")}' + '_is1')
end;


function GetUninstallExePath: string;
var
  sUnInstPath: string;
  sUnInstallString: String;
begin
  Result := '';
  sUnInstPath := GetUninstallRegPath(); { Your App GUID/ID }
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;


function InitializeSetup: Boolean;
var
  V: Integer;
  iResultCode: Integer;
  sUnInstallString: string;
begin        
  
  Result := True
  while RegValueExists(HKEY_LOCAL_MACHINE, GetUninstallRegPath(), 'UninstallString') do
  begin
    V := MsgBox(ExpandConstant('Hey! Another version of the game was detected. Do you want to uninstall it?'), mbInformation, MB_YESNO);
    if V = IDYES then
    begin
      sUnInstallString := GetUninstallExePath();
      sUnInstallString := RemoveQuotes(sUnInstallString);
      Exec(ExpandConstant(sUnInstallString), '', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
    end
    else
    begin
      MsgBox(ExpandConstant('Well, go play it since it is installed already! :)'), mbInformation, MB_OK);
      Result := False;
      break;
    end;
  end; 

end;



[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\build\windows_x64\dnc_win_x64.exe"; DestDir: "{app}";
Source: "..\build\windows_x64\game.dll"; DestDir: "{app}"; 
Source: "..\build\windows_x64\renderer.dll"; DestDir: "{app}"; 
Source: "..\build\windows_x64\yuu"; DestDir: "{app}"; 
Source: "..\icons\window.ico"; DestDir: "{app}"; 
Source: "..\icons\setup.ico"; DestDir: "{app}"; 

[Icons]
Name: {group}\momodevelop\Dots and Circles; Filename: {app}\dnc_win_x64.exe; WorkingDir: {app}; IconFilename: "{app}\setup.ico"
Name: {commondesktop}\Dots and Circles; Filename: {app}\dnc_win_x64.exe; Tasks: desktopicon; WorkingDir: {app}; IconFilename: "{app}\setup.ico"


[Run]
Filename: {app}\dnc_win_x64.exe; Description: Run Application; Flags: postinstall nowait skipifsilent unchecked