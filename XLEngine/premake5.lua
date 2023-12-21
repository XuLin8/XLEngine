project "XLEngine" --��Ŀ����
	kind "StaticLib" --��������Ŀ�Ǿ�̬���ӿ�
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")--���Ŀ¼
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")--�м���ʱ�ļ���Ŀ¼

	pchheader "xlpch.h"
	pchsource "src/xlpch.cpp"

	files--����Ŀ���ļ�
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

	includedirs--���Ӱ���Ŀ¼
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

	filter "system:windows"--windowsƽ̨������
		systemversion "latest"
		characterset ("MBCS")  

		defines--Ԥ�����
		{
			"XL_PLATFORM_WINDOWS",
			"XL_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE",
		}
		--postbuildcommands -- build����Զ�������
        --{
        --    ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox\"") --��������dll�⵽sanbox.exe��ͬһĿ¼��ȥ
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