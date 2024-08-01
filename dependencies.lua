target = string.lower(os.target())  -- Get the target platform (windows, linux, etc.)

VULKAN_SDK = os.getenv("VULKAN_SDK")  -- Get the VulkanSDK path from the environment variables

--[[
	The dependencies table is used to specify the dependencies of the project.
	Here is an example of adding a dependency:
	
	dependencyName = {
		libName = "libName",
		libDir = "path_to_lib_dir",
		includeDir = "path_to_include_dir",
		windows = {
			debugLibName = "debugLibName",
		},
		configs = "Debug,Release"
	}

	Description of the fields:
	- libName: The name of the dependency library file.
	- libDir: Specifies where the lib file is located.
	- includeDir: The path to the include directory of the dependency.
	- windows: A scope for Windows-specific configurations. Linux, MacOS, etc. can be added in the same way.
	- debugLibName: Some dependencies have different names for debug and release builds. This field is used to specify the debug library name.
	- configs: A list of configurations that the dependency should be used in.
]]--
dependencies = {
	vulkan = {
		windows = {
			libName = "vulkan-1",
			libDir = "%{VULKAN_SDK}/Lib/",
		},
		includeDir = "%{VULKAN_SDK}/Include/",
	},
	glfw = {
		libName = "GLFW",
		includeDir = "%{wks.location}/Astranox/vendor/glfw/include",
	},
	glm = {
		includeDir = "%{wks.location}/Astranox/vendor/glm",
	},
}


-- Functions for dependencies >>>
function _linkDependency(libDict, isDebug)
	if libDict.libDir ~= nil then
		libdirs { libDict.libDir }
	end

	-- Fetch the library name >>>
	local libName = nil
	if libDict.libName ~= nil then
		libName = libDict.libName
	end

	-- If the configuration is debug and the debug library name is specified, use that for linking
	if isDebug and libDict.debugLibName ~= nil and target == "windows" then
		libName = libDict.debugLibName
	end
	-- <<< Fetch the library name

	-- Link the library
	if libName ~= nil then
		links { libName }
		return true
	end

	return false  -- Indicates that the dependency was not linked
end


function _addDependencyIncludeDirs(libDict)
	if libDict.includeDir ~= nil then
		externalincludedirs { libDict.includeDir }
	end
end


function processDependencies(configName)
	for key, libDict in pairs(dependencies) do
		local configMatched = true

		-- If configuration is specified, only match the configuration
		if configName ~= nil and libDict.configs ~= nil then
			configMatched = string.find(libDict.configs, configName)
		end

		if configMatched then
			local isDebug = configName == "Debug"
			local linkSucceeded = false

			-- Try to link the platform-specific scope
			if libDict[target] ~= nil then
				linkSucceeded = _linkDependency(libDict[target], isDebug)
				_addDependencyIncludeDirs(libDict[target])
			end

			-- If the platform-specific scope was not linked, link the global scope
			if not linkSucceeded then
				_linkDependency(libDict, isDebug)
				_addDependencyIncludeDirs(libDict)
			end
		end

	end
end

function includeDependencies(configName)
	for key, libDict in pairs(dependencies) do
		local configMatched = true

		-- If there is a configuration list, only match the configuration
		if configName ~= nil and libDict.configs ~= nil then
			configMatched = string.find(libDict.configs, configName)
		end

		if configMatched then
			-- Include global scope
			_addDependencyIncludeDirs(libDict)

			-- Include platform-specific scope
			if libDict[target] ~= nil then
				_addDependencyIncludeDirs(libDict[target])
			end
		end

	end
end
-- <<< Functions for dependencies