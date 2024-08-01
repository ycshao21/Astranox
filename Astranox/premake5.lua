project "Astranox"
	kind "StaticLib"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.hpp"
	pchsource "src/pch.cpp"

	files {
		"include/**.h",
		"include/**.hpp",
		"src/**.c",
		"src/**.cpp",
	}

	includedirs {
        "include/",
        "vendor/",
    }

	includeDependencies()

	defines {
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",  -- Vulkan uses [0, 1] depth range
	}

	filter "system:windows"
		systemversion "latest"
		defines {
            "AW_PLATFORM_WINDOWS",
        }

	filter "configurations:Debug"
		symbols "On"
		defines {
            "AW_CONFIG_DEBUG",
        }

	filter "configurations:Release"
		optimize "On"
		defines {
            "AW_CONFIG_RELEASE",
        }
	