project "Astranox-Rasterization"
	kind "ConsoleApp"
	staticruntime "off"

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
		"%{dependencies.stb_image.includeDir}",
	}

	includeDependencies()

	filter "system:windows"
		systemversion "latest"
		defines {
            "AST_PLATFORM_WINDOWS"
        }

	filter "configurations:Debug"
		symbols "On"
		defines {
            "AST_CONFIG_DEBUG",
        }
		processDependencies("Debug")

	filter "configurations:Release"
		optimize "On"
		defines {
			"AST_CONFIG_RELEASE"
		}
		processDependencies("Release")