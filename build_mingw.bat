mkdir build_mingw
cd build_mingw 
cmake .. -G "MinGW Makefiles"
mingw32-make.exe package
cd ..