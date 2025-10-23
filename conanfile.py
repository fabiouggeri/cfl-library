from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class cfl_libraryRecipe(ConanFile):
    name = "cfl-library"
    version = "1.9.0"
    package_type = "library"

    # Optional metadata
    license = "ASF 2.0"
    author = "Fabio Uggeri fabiouggeri@gmail.com"
    url = "https://github.com/fabiouggeri/cfl-library"
    description = "Commons functions library is a set of functions commonly used in my C projects. The goal is to be an abstraction and simplification layer."
    topics = ("c", "commons", "library")
    #generators = "CMakeToolchain", "CMakeDeps"

    # requires = "zlib/1.2.13" #, "fmt/10.1.1"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "cfl-headers/**", "cfl-lib/**"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["cfl-lib"]

