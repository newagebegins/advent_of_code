@echo off

set BuildDir=build
if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%

REM cl /Od /D_CRT_SECURE_NO_WARNINGS /Oi /Z7 /Zo /MTd /fp:fast /fp:except- /nologo /FC /GR- /Gm- /EHa- /W4 /WX /wd4100 /wd4189 /wd4201 /wd4505 ..\main.cpp /link /opt:ref /incremental:no

set CommonCompilerFlags=/Od /Oi /Z7 /Zo /MTd /fp:fast /fp:except- /nologo /FC /GR- /Gm- /EHa- /W4 /WX /wd4100 /wd4189 /wd4201 /wd4505
set CommonLinkerFlags=/opt:ref /incremental:no

cl %CommonCompilerFlags% /std:c++20 /EHsc ..\main.cpp /link %CommonLinkerFlags%

popd
