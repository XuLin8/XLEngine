#pragma once

#include "Runtime/Core/Base/Base.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/Entity/Entity.h"

namespace XLEngine
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Level>& context);

        void SetContext(const Ref<Level>& context);

        void OnImGuiRender(bool* pOpen, bool* pOpenProperties);

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(Entity entity);
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
    private:
        Ref<Level> m_Context;
        Entity m_SelectionContext;
    };
}