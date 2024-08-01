project "Astranox-Rasterization"
	kind "ConsoleApp"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	links  { "Astranox" }

	files {
		"include/**.h", 
		"include/**.hpp", 
		"src/**.c",
		"src/**.cpp" 
	}

	includedirs {
		"include/",
		"../Astranox/include",
		"../Astranox/vendor/",
	}

	filter "system:windows"
		systemversion "latest"
		defines {
            "AW_PLATFORM_WINDOWS"
        }

	filter "configurations:Debug"
		symbols "On"
		defines {
            "AW_CONFIG_DEBUG",
        }
		processDependencies("Debug")

	filter "configurations:Release"
		optimize "On"
		defines {
			"AW_CONFIG_RELEASE"
		}
		processDependencies("Release")