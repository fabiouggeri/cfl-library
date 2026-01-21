rm -r -f build-gcc
mkdir build-gcc
cmake -B build-gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS=-lm
cmake --build build-gcc --config Release

