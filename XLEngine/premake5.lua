project "XLEngine" --项目名称
	kind "StaticLib" --表明该项目是静态链接库
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")--输出目录
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")--中间临时文件的目录

	pchheader "xlpch.h"
	pchsource "src/xlpch.cpp"

	files--该项目的文件
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.h",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

	includedirs--附加包含目录
	{
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		
	}

	links
	{
		"Box2D",
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		
	}

	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"--windows平台的配置
		systemversion "latest"
		characterset ("MBCS")  

		defines--预编译宏
		{
			"XL_PLATFORM_WINDOWS",
			"XL_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE",
		}
		--postbuildcommands -- build后的自定义命令
        --{
        --    ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox\"") --拷贝引擎dll库到sanbox.exe的同一目录下去
        --}

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