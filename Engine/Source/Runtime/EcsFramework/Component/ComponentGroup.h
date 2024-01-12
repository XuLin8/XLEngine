#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/EcsFramework/Component/Basic/IDComponent.h"
#include "Runtime/EcsFramework/Component/Basic/TagComponent.h"
#include "Runtime/EcsFramework/Component/Camera/CameraComponent.h"
#include "Runtime/EcsFramework/Component/Physics/BoxCollider2DComponent.h"
#include "Runtime/EcsFramework/Component/Physics/CircleCollider2DComponent.h"
#include "Runtime/EcsFramework/Component/Physics/Rigidbody2DComponent.h"
#include "Runtime/EcsFramework/Component/Script/NativeScriptComponent.h"
#include "Runtime/EcsFramework/Component/Shape/CircleRendererComponent.h"
#include "Runtime/EcsFramework/Component/Shape/SpriteRendererComponent.h"
#include "Runtime/EcsFramework/Component/Transform/TransformComponent.h" 
#include "Runtime/EcsFramework/Component/Mesh/StaticMeshComponent.h" 

#include <concepts>
#include <type_traits>

namespace XLEngine
{
    // Every Component Class should be registered in this file
    
    // 定义一个 Concept 用于检查是否为组件类型
    template<typename T>
    concept Component = std::is_base_of_v<ComponentBase, T>;

    // 使用 Concepts 约束 ComponentGroup，要求所有模板参数必须是组件类型
    template<Component... Components>
    struct ComponentGroup
    {
        // 可以在这里添加 ComponentGroup 的具体实现
       
    };
    using AllComponents = ComponentGroup<TransformComponent, CircleRendererComponent, SpriteRendererComponent,
        CameraComponent, NativeScriptComponent,
        Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
        StaticMeshComponent>;
}