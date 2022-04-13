
project "Chip8-Emulator"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir (outputTargetDir)
    objdir (outputObjDir)

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glad}"
    }

    links {
        "%{Library.glfw}",
        "%{Library.glad}",
        "opengl32.lib"
    }

    defines { "GLFW_INCLUDE_NONE" }

    filter { "configurations:Debug" }
        symbols "On"
        defines {
            "LOGGING_ENABLED",
            "ASSERTS_ENABLED"
        }

    filter { "configurations:Release" }
        symbols "On"
        optimize "On"
        defines {
            "LOGGING_ENABLED",
            "ASSERTS_ENABLED"
        }

    filter { "configurations:Dist" }
        optimize "On"