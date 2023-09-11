workspace "XLEngine" --�����������
    architecture "x64" --����ƽ̨ ֻ��64λ--(x86_64,ARM)

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }
--��ʱ���� ���� ���Ŀ¼
--��ϸ������֧�ֵ�tokens �ɲο� [https://github.com/premake/premake-core/wiki/Tokens]
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "XLEngine" --��Ŀ����
    location "XLEngine" --���·��
    kind "SharedLib" --��������Ŀ��dll��̬��
    language "c++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")--���Ŀ¼
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")--�м���ʱ�ļ���Ŀ¼

    files--����Ŀ���ļ�
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs--���Ӱ���Ŀ¼
    {
        "%{prj.name}/vendor/spdlog/include"
    }

    filter "system:windows"--windowsƽ̨������
        cppdialect "c++17"
        staticruntime "On"
        systemversion "latest"

        defines --Ԥ�����
        {
            "XL_BUILD_DLL",
            "XL_PLATFORM_WINDOWS",
        }

        postbuildcommands -- build����Զ�������
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox") --��������dll�⵽sanbox.exe��ͬһĿ¼��ȥ
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
    language "c++"

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
        "XLEngine/src"
    }

    links
    {
        "XLEngine"
    }

    filter "system:windows"
        cppdialect "c++17"
        staticruntime "On"
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