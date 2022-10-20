@echo off

SET PATH=%PATH%;%~dp0\bin

SET me=%~dp0

SET RootDir=%me%
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build\windows_x64

rd /s /q %BuildDir%
mkdir %BuildDir%
pushd %BuildDir%


rem Prepare Windows x64 Environment -------------------

call setup_cl_x64.bat

rem Build Assets --------------------------------------
echo --- BUILDING ASSETS ---------------------------- 
call build_assets.bat
echo Copying asset file over
xcopy %RootDir%\build\yuu . /Q /Y

rem Build Renderer DLL --------------------------------
echo --- BUILDING RENDERER ---------------------------- 

SETLOCAL

SET CommonCompilerFlags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100  -GR -EHa -INCREMENTAL:NO -std:c++17 
SET CommonCompilerFlags=%CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref -EXPORT:Renderer_Init -EXPORT:Renderer_BeginFrame -EXPORT:Renderer_EndFrame -EXPORT:Renderer_Free -EXPORT:Renderer_AddTexture -EXPORT:Renderer_ClearTextures
SET CommonLinkerFlags=user32.lib opengl32.lib gdi32.lib  %CommonLinkerFlags%

cl %CommonCompilerFlags% %CodeDir%\win32_renderer_opengl.cxx -LD -link %CommonLinkerFlags% /OUT:renderer.dll

ENDLOCAL

rem Build Game DLL -------------------------------------
echo --- BUILDING GAME DLL ---------------------------- 

SETLOCAL

SET CommonCompilerFlags=-O2 -MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100   -GR -EHa -INCREMENTAL:NO -std:c++17
SET CommonCompilerFlags=%CommonCompilerFlags%

cl %CommonCompilerFlags% %CodeDir%\game.cxx -LD -link -EXPORT:Game_Update -OUT:game.dll



ENDLOCAL

rem Build Exe -----------------------------
echo --- BUILDING PLATFORM ---------------------------- 

SETLOCAL

SET CommonCompilerFlags=-O2 -MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -GR -EHa -INCREMENTAL:NO -std:c++17
SET CommonCompilerFlags=%CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref
SET CommonLinkerFlags=user32.lib opengl32.lib gdi32.lib winmm.lib ole32.lib imm32.lib shell32.lib %CommonLinkerFlags%

cl %CommonCompilerFlags% %CodeDir%\win32.cxx -link %CommonLinkerFlags% -OUT:dnc_win_x64.exe


ENDLOCAL


rem Cleanup -----------------------------------
echo --- CLEANUP ---------------------------- 
del /S *.exp
del /S *.lib
del /S *.obj


rem Copy icon for window ----------------------
echo --- COPYING ICON FOR WINDOWS ---------------------------- 
xcopy %RootDir%\icons\window.ico . /Q /Y


popd


rem Create installer --------------------------
echo --- CREATING INSTALLER ---------------------------- 
iscc ".\\inno\\gen_x64_installer.iss"

