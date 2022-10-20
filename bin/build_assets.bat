@echo off

SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1%

call bin\setup_cl_x64.bat
IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)


SET CommonCompilerFlags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

pushd %BuildDir%

cl %CommonCompilerFlags% %CodeDir%\tool_build_assets.cxx

if NOT %ERRORLEVEL% == 0 goto exit
del /Q %cd%\assets
if not exist "%cd%\assets" mkdir %cd%\assets
copy %RootDir%\assets\* %cd%\assets
call tool_build_assets.exe

popd
