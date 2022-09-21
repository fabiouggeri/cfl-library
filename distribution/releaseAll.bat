SET GOAL=%1

IF "%GOAL%"=="" SET GOAL=install

call mvn %GOAL%
call mvn %GOAL% -Darch=32
call mvn %GOAL% -Dcc=bcc
call mvn %GOAL% -Dcc=mingw64
call mvn %GOAL% -Dcc=mingw64 -Darch=32
rem mingw64 is compatible with clang-gcc
rem call mvn %GOAL% -Dcc=clang
rem call mvn %GOAL% -Dcc=clang -Darch=32
