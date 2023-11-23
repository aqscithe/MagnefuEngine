#pragma once

#include "Magnefu/Scene/Scene.h"
#include "entt.hpp"

namespace Magnefu
{

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, Scene* scene, std::string& name) :
			m_EntityHandle(entity), m_Scene(scene), m_Name(name)
		{}

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		~Entity();

		inline const std::string& GetName() const { return m_Name; }

		void* GetTransformComponent();
		void* GetMeshComponent();

		template<typename T>
		bool Contains()
		{
			return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
		}
		

		template<typename T, typename...Args>
		T& AddComponent(Args&&...args)
		{
			return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T, typename...Args>
		T& ReplaceComponent(Args&&...args)
		{
			return m_Scene->GetRegistry().emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		/*template<typename T, typename...Args>
		T& TryAddComponent(Args&&...args)
		{
			return m_Scene->GetRegistry().try_emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}*/

	private:
		std::string m_Name;
		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}