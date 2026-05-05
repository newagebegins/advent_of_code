@echo off

set BuildDir=build
if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%

cl /Od /Oi /Z7 /Zo /MTd /fp:fast /fp:except- /nologo /FC /GR- /Gm- /EHa- /W4 /WX /wd4100 /wd4189 /wd4201 /wd4505 ..\main.cpp /link /opt:ref /incremental:no

popd
