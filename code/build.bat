@echo off

mkdir ..\build
pushd ..\build

rem -Zi -- generate debug info
rem -FC -- use full paths

cl -FC -Zi ..\code\win32_handmade.cpp User32.lib Gdi32.lib

popd