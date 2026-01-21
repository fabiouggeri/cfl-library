@echo off
rem -----------
conan create . --build=* --profile=mingw_profile
rem conan remove "cfl-library/1.10.0"
rem conan create . --build=missing --profile=mingw_profile

rem -----------
rem cmake .. -G "Visual Studio 15 2017" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"

rem -----------
rem mkdir build_mingw
rem conan install . --output-folder=build_mingw --build=missing --profile=mingw_profile
rem cd build_mingw
rem cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
rem cd ..

rem -----------
rem mkdir build_mingw
rem conan install . --output-folder=build_mingw --build=missing --profile=mingw_profile
rem cd build_mingw
rem cmake .. -G "MinGW Makefiles"
rem cmake --build .
rem ctest
rem cd ..