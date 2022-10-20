@echo off
    
SET me=%~dp0
    

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

if exist tool_build_assets.rdbg (
	call remedybg tool_build_assets.rdbg
) else (
	call remedybg tool_build_assets.exe
)


popd
