#pragma once

//��Ӧ�ó���ʹ��
#include "Runtime/Core/Application.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/Input.h"
#include "Runtime/Core/KeyCodes.h"
#include "Runtime/Core/MouseCodes.h"
#include "Runtime/Core/Timestep.h"
#include "Runtime/Renderer/OrthographicCameraController.h"
#include "Runtime/ImGui/ImGuiLayer.h"

#include "Runtime/Scene/Scene.h"
#include "Runtime/Scene/Entity.h"
#include "Runtime/Scene/ScriptableEntity.h"

#include "Runtime/Scene/Component.h"

#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"

//Rnderer
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/Renderer2D.h"
#include "Runtime/Renderer/RenderCommand.h"
#include "Runtime/Renderer/Buffer.h"
#include "Runtime/Renderer/Shader.h"
#include "Runtime/Renderer/Framebuffer.h"
#include "Runtime/Renderer/Texture.h"
#include "Runtime/Renderer/SubTexture2D.h"
#include "Runtime/Renderer/VertexArray.h"
#include "Runtime/Renderer/OrthographicCamera.h"