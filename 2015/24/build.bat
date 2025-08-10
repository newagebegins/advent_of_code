@echo off
mkdir build
pushd build
cl /O2 /FC /GR- /EHa- /nologo /Zi ../main.cpp
popd