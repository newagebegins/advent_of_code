@echo off

set BuildDir=build
if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%

set CommonCompilerFlags=/Od /Oi /Z7 /Zo /MTd /fp:fast /fp:except- /nologo /FC /GR- /Gm- /EHa- /W4 /WX /wd4100 /wd4189 /wd4201 /wd4505 -D_CRT_SECURE_NO_WARNINGS
set CommonLinkerFlags=/opt:ref /incremental:no

cl %CommonCompilerFlags% ..\main.cpp /link %CommonLinkerFlags%

popd
