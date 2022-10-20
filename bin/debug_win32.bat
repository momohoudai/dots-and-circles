@echo off
SET me=%~dp0
SET RootDir=%me%\..
SET BuildDir=%RootDir%\build
SET Opt=%1%



pushd %BuildDir%


if exist win32.rdbg (
	call remedybg win32.rdbg
) else (
	call remedybg win32.exe
)

popd
