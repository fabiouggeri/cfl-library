#!/bin/bash
set -e

# Check if compiler argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 [gcc|clang]"
    exit 1
fi

COMPILER=$1

# Set compiler-specific variables
if [ "$COMPILER" == "gcc" ]; then
    CC_COMPILER="gcc"
    CXX_COMPILER="g++"
    CONAN_PROFILE=./.conan/gcc13_profile
elif [ "$COMPILER" == "clang" ]; then
    CC_COMPILER="clang"
    CXX_COMPILER="clang++"
    CONAN_PROFILE=./.conan/clang18_profile
else
    echo "Unsupported compiler: $COMPILER. Use 'gcc' or 'clang'."
    exit 1
fi

if [ "$2" == "install" ]; then
   echo "Creating Conan package..."
   conan create . --build=missing --profile=$CONAN_PROFILE
   echo "Package creation for $COMPILER completed successfully."
   exit 0
fi

BUILD_DIR="build-$COMPILER"

echo "Building with $COMPILER in directory $BUILD_DIR..."

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"

# Configure
cmake -B "$BUILD_DIR" \
    -DCMAKE_C_COMPILER="$CC_COMPILER" \
    -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS=-lm

# Build (this also runs tests due to auto_test target)
cmake --build "$BUILD_DIR" --config Release

# Package
echo "Generating ZIP package..."
cd "$BUILD_DIR"
cpack -G ZIP
cd ..

echo "Build and packaging for $COMPILER completed successfully."
