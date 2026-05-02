@echo off

set BuildDir=build
if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%

set CommonCompilerFlags=/O2 /Oi /Z7 /Zo /MTd /fp:fast /fp:except- /nologo /FC /GR- /Gm- /EHa- /W4 /WX /wd4100 /wd4189 /wd4201 /wd4505
set CommonLinkerFlags=/opt:ref /incremental:no

cl %CommonCompilerFlags% /std:c++20 /EHsc ..\main.cpp /link %CommonLinkerFlags%

popd
