
include "Dependencies.lua"

outputTargetDir = "Build/%{cfg.buildcfg}-%{cfg.platform}/Bin"
outputObjDir = "Build/%{cfg.buildcfg}-%{cfg.platform}/Obj"

workspace "Chip8-Emulator"
    startproject "Chip8-Emulator"
    configurations { "Debug", "Release", "Dist" }
    platforms { "Win64" }
    flags { "MultiProcessorCompile" }

    filter { "platforms:Win64" }
        system "windows"
        architecture "x86_64"

    filter { "system:windows" }
        systemversion "latest"

include "Chip8-Emulator"
include "ImGui"
