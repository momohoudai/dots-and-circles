@echo off

SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1

if not exist %BuildDir% mkdir %BuildDir%


SET CommonCompilerFlags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -GR -EHa -std:c++17
SET CommonCompilerFlags=-DINTERNAL  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref
SET CommonLinkerFlags=user32.lib opengl32.lib gdi32.lib winmm.lib ole32.lib imm32.lib shell32.lib %CommonLinkerFlags%

pushd %BuildDir%

cl %CommonCompilerFlags% %CodeDir%\win32.cxx -link %CommonLinkerFlags%


rem Copy icon for window ----------------------
xcopy %RootDir%\icons\window.ico  %BuildDir% /Q /Y

popd
