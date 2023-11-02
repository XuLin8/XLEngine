workspace "XLEngine" --�����������
    architecture "x64" --����ƽ̨ ֻ��64λ--(x86_64,ARM)
    startproject "XLEngineEditor"

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }
    flags
	{
		"MultiProcessorCompile"
	}
--��ʱ���� ���� ���Ŀ¼
--��ϸ������֧�ֵ�tokens �ɲο� [https://github.com/premake/premake-core/wiki/Tokens]
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "XLEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "XLEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "XLEngine/vendor/imgui"
IncludeDir["glm"] = "XLEngine/vendor/glm"
IncludeDir["stb_image"] = "XLEngine/vendor/stb_image"
IncludeDir["entt"] = "XLEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "XLEngine/vendor/yaml-cpp/include"

group "Dependencies"
    include "XLEngine/vendor/GLFW"
    include "XLEngine/vendor/Glad"
    include "XLEngine/vendor/imgui"
    include "XLEngine/vendor/yaml-cpp"

group ""

project "XLEngine" --��Ŀ����
    location "XLEngine" --���·��
    kind "StaticLib" --��������Ŀ�Ǿ�̬���ӿ�
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")--���Ŀ¼
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")--�м���ʱ�ļ���Ŀ¼

    pchheader "xlpch.h"
    pchsource "XLEngine/src/xlpch.cpp"

    files--����Ŀ���ļ�
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
    }
    
    defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

    includedirs--���Ӱ���Ŀ¼
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}",
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "yaml-cpp",
        "opengl32.lib"
    }

    filter "system:windows"--windowsƽ̨������
        systemversion "latest"

        defines --Ԥ�����
        {
            "XL_BUILD_DLL",
            "XL_PLATFORM_WINDOWS",
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
        runtime "Debug"
        optimize "on"

    filter "configurations:Dist"
        defines "XL_DIST"
        runtime "Release"
        optimize "on"

project "XLEngineEditor"
    location "XLEngineEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "XLEngine/vendor/spdlog/include",
        "XLEngine/src",
        "XLEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}"
    }

    links
    {
        "XLEngine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "XL_PLATFORM_WINDOWS",
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

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "XLEngine/vendor/spdlog/include",
        "XLEngine/src",
        "XLEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}"
    }

    links
    {
        "XLEngine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "XL_PLATFORM_WINDOWS",
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