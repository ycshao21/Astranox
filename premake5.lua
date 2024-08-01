include "dependencies.lua"

workspace "Astranox"
    architecture "x64"

	targetdir "build"
	startproject "Astranox-Rasterization"

	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	defines {
		"_CRT_SECURE_NO_WARNINGS",
	}

	configurations {
        "Debug",
        "Release",
    }

	filter "configurations:Debug"
		optimize "Off"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		symbols "On"

	filter "system:windows"	
		buildoptions {
            "/Zc:__cplusplus"
        }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Core"
    include "Astranox"

group "Runtime"
    include "Astranox-Rasterization"

group "Dependencies"
	include "Astranox/vendor/glfw"