@echo off
if not exist build mkdir build
pushd build
cl /nologo /Zi /W4 /WX ..\main.cpp
popd
