conan create . --build=missing --profile=mingw_profile
rem mkdir build_mingw
rem cd build_mingw 
rem cmake .. -G "MinGW Makefiles"
rem mingw32-make.exe package
rem cd ..