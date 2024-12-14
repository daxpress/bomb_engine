from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conan.tools.env import Environment, VirtualBuildEnv


class BombEngineRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    build_policy = "missing"
    # "imgui/cci.20230105+1.89.2.docking", "imguizmo/cci.20231114", "assimp/5.2.2", "fmt/10.0.0", "clove-unit/2.4.0"
    requires = "glfw/3.4", "glm/0.9.9.8", "spirv-cross/1.3.268.0", \
        "entt/3.13.0", "tinyobjloader/2.0.0-rc10", "stb/cci.20240213", \
        "fmt/11.0.2", "pybind11/2.13.6"

    def generate(self):
        # Use STATIC_ANALYSIS buildenv variable in conan profile to enable static analysis
        ms = VirtualBuildEnv(self)
        ms.generate()
        static_analysis = ms.environment().vars(self).get("STATIC_ANALYSIS", False)
        tc = CMakeToolchain(self)
        tc.cache_variables["STATIC_ANALYSIS"] = static_analysis
        tc.generate()

    def layout(self):
        self.folders.build_folder_vars = ["settings.os", "settings.compiler", "settings.compiler.version",
                                          "settings.arch", "settings.build_type"]

        ms = VirtualBuildEnv(self)
        static_analysis = ms.environment().vars(self).get("STATIC_ANALYSIS", False)
        if static_analysis:
            self.folders.build_folder_vars.append("const.analysis")

        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
