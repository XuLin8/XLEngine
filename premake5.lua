workspace "XLEngine" --解决方案名称
    architecture "x64" --编译平台 只编64位--(x86_64,ARM)

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }
--临时变量 定义 输出目录
--详细的所有支持的tokens 可参考 [https://github.com/premake/premake-core/wiki/Tokens]
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "XLEngine" --项目名称
    location "XLEngine" --相对路径
    kind "SharedLib" --表明该项目是dll动态库
    language "c++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")--输出目录
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")--中间临时文件的目录

    files--该项目的文件
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs--附加包含目录
    {
        "%{prj.name}/vendor/spdlog/include"
    }

    filter "system:windows"--windows平台的配置
        cppdialect "c++17"
        staticruntime "On"
        systemversion "latest"

        defines --预编译宏
        {
            "XL_BUILD_DLL",
            "XL_PLATFORM_WINDOWS",
        }

        postbuildcommands -- build后的自定义命令
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox") --拷贝引擎dll库到sanbox.exe的同一目录下去
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