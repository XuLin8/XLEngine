include "./vendor/premake/premake_customization/solution_items.lua"

workspace "XLEngine" --�����������
    architecture "x86_64" --����ƽ̨ ֻ��64λ--(x86_64,ARM)
    startproject "XLEngineEditor"

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items
	{
		".editorconfig"
	}

    flags
	{
		"MultiProcessorCompile"
	}
--��ʱ���� ���� ���Ŀ¼
--��ϸ������֧�ֵ�tokens �ɲο� [https://github.com/premake/premake-core/wiki/Tokens]
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] =        "%{wks.location}/XLEngine/vendor/GLFW/include"
IncludeDir["Glad"] =        "%{wks.location}/XLEngine/vendor/Glad/include"
IncludeDir["ImGui"] =       "%{wks.location}/XLEngine/vendor/imgui"
IncludeDir["glm"] =         "%{wks.location}/XLEngine/vendor/glm"
IncludeDir["stb_image"] =   "%{wks.location}/XLEngine/vendor/stb_image"
IncludeDir["entt"] =        "%{wks.location}/XLEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] =    "%{wks.location}/XLEngine/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] =	"%{wks.location}/XLEngine/vendor/ImGuizmo"
IncludeDir["Box2D"] =	    "%{wks.location}/XLEngine/vendor/Box2D/include"

group "Dependencies"
    include "vendor/premake"
    include "XLEngine/vendor/Box2D"
    include "XLEngine/vendor/GLFW"
    include "XLEngine/vendor/Glad"
    include "XLEngine/vendor/imgui"
    include "XLEngine/vendor/yaml-cpp"

group ""

include "XLEngine"
include "Sandbox"
include "XLEngineEditor"