project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
	}

	includedirs 
	{
		"%{wks.location}/XLEngine/vendor/spdlog/include",
		"%{wks.location}/XLEngine/src",
		"%{wks.location}/XLEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
	}

	links
	{
		"XLEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"XL_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "XL_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "XL_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "XL_DIST"
		runtime "Release"
		optimize "on"