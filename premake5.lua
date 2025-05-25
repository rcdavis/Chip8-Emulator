
include "Dependencies.lua"

outputTargetDir = "Build/%{cfg.buildcfg}-%{cfg.platform}/Bin"
outputObjDir = "Build/%{cfg.buildcfg}-%{cfg.platform}/Obj"

workspace "Chip8-Emulator"
    startproject "Chip8-Emulator"
    configurations { "Debug", "Release", "Dist" }
    platforms { "Linux", "Win64" }
    flags { "MultiProcessorCompile" }
    architecture "x86_64"

    filter { "platforms:Win64" }
        system "windows"
        systemversion "latest"

    filter { "platforms:Linux" }
        system "linux"

include "Chip8-Emulator"
include "ImGui"
