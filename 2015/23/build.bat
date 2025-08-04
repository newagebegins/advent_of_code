@echo off
mkdir build
pushd build
cl /FC /GR- /EHa- /nologo /Zi ../main.cpp
popd