SET GOAL=%1

IF "%GOAL%"=="" SET GOAL=install

call mvn clean

call mvn %GOAL% -Drelease
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Darch=32
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Dcc=bcc
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Dcc=mingw64
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Dcc=mingw64 -Darch=32
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Dcc=clang
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

call mvn %GOAL% -Drelease -Dcc=clang -Darch=32
IF NOT "%ERRORLEVEL%"=="0" GOTO ERRO

GOTO FIM

:ERRO
echo "Error building RGT"

:FIM