
IncludeDir = {}
IncludeDir["Spdlog"] = "%{wks.location}/Chip8-Emulator/vendor/Spdlog/src"
IncludeDir["glfw"] = "%{wks.location}/Chip8-Emulator/vendor/glfw/src"
IncludeDir["glad"] = "%{wks.location}/Chip8-Emulator/vendor/glad/src"

Library = {}
Library["glfw"] = "%{wks.location}/Chip8-Emulator/vendor/glfw/lib/%{cfg.platform}/glfw3_mt.lib"
Library["glad"] = "%{wks.location}/Chip8-Emulator/vendor/glad/lib/%{cfg.platform}/Glad.lib"
