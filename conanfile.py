from conans import ConanFile, CMake, tools


class Pod5Conan(ConanFile):
    name = "pod5_file_format"
    license = "MPL 2.0"
    url = "https://github.com/nanoporetech/pod5-file-format"
    description = "POD5 File format"
    topics = "arrow"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "nanopore_internal_build": [True, False]}
    default_options = {
        "shared": False,
        "nanopore_internal_build": False,
    }
    generators = "cmake_find_package_multi"
    exports_sources = [
        "c++/*",
        "cmake/*",
        "python/*",
        "third_party/*",
        "CMakeLists.txt",
        "LICENSE.md",
    ]

    def requirements(self):
        package_suffix = ""
        if self.options.nanopore_internal_build:
            package_suffix = "@nanopore/stable"

        self.requires(f"arrow/7.0.0{package_suffix}")
        self.requires(f"boost/1.78.0{package_suffix}")
        self.requires(f"flatbuffers/2.0.0{package_suffix}")
        self.requires(f"zstd/1.4.8{package_suffix}")

    def build(self):
        cmake = CMake(self)
        shared = "ON" if self.options.shared else "OFF"
        self.run(
            f"cmake . -DUSE_CONAN=ON -DBUILD_PYTHON_WHEEL=OFF -DINSTALL_THIRD_PARTY=OFF {cmake.command_line} -DBUILD_SHARED_LIB={shared}"
        )
        self.run(f"cmake --build . {cmake.build_config}")

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
