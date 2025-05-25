
project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    staticruntime "off"

	targetdir (outputTargetDir)
    objdir (outputObjDir)

	files
	{
		"src/*.h",
		"src/*.cpp"
	}

	includedirs {
        "src",
    }

	filter { "platforms:Linux" }
        includedirs {
            "/usr/include"
        }

        libdirs {
            "/usr/lib/x86_64-linux-gnu"
        }

        links {
            "glfw"
        }

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
