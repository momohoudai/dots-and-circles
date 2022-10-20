@echo off
echo "Entering development mode"
PATH=%PATH%;%~dp0\bin
bin\setup_cl_x64.bat && 4ed