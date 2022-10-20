@echo off

SET RootDir=%me%..
SET SandboxDir=%RootDir%\sandbox\%1

pushd %SandboxDir%

if exist test.rdbg (
	call remedybg test.rdbg
) else (
	call remedybg test.exe
)



:End 

popd
