from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake


class BombEngineRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    build_policy = "missing"
    # "imgui/cci.20230105+1.89.2.docking", "imguizmo/cci.20231114", "assimp/5.2.2", "fmt/10.0.0", "clove-unit/2.4.0"
    requires = "glfw/3.4", "glm/0.9.9.8", "spirv-cross/1.3.268.0", \
        "entt/3.13.0", "tinyobjloader/2.0.0-rc10", "stb/cci.20240213", \
        "fmt/11.0.2", "pybind11/2.13.6"

    def layout(self):
        self.folders.build_folder_vars = ["settings.os", "settings.compiler", "settings.compiler.version",
                                          "settings.arch", "settings.build_type"]
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
