#pragma once
#include "Runtime/Core/UUID.h"

#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"
#include <entt.hpp>

#include <tuple>
#include <type_traits>

namespace XLEngine
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Level* scene);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            XL_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            XL_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename... T>
        std::tuple<T*...> GetComponents()
        {
            XL_CORE_ASSERT((HasComponent<T>() && ...), "Entity does not have component!");
            return std::make_tuple<T*...>((&m_Scene->m_Registry.get<T>(m_EntityHandle))...);
        }

        template<typename... T>
        std::tuple<const T* const...> GetConstComponents()
        {
            XL_CORE_ASSERT((HasComponent<T>() && ...), "Entity does not have component!");
            return std::make_tuple(((const T* const)&m_Scene->m_Registry.get<T>(m_EntityHandle))...);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            XL_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }
        const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }
    private:
        entt::entity m_EntityHandle{ entt::null };
        Level* m_Scene = nullptr;
    };
}
