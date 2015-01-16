@echo off

mkdir ..\build
pushd ..\build

rem -Zi -- generate debug info

cl -Zi ..\code\win32_handmade.cpp User32.lib Gdi32.lib

popd