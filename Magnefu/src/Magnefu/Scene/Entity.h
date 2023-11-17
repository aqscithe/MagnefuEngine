#pragma once

#include "Magnefu/Scene/Scene.h"
#include "entt.hpp"

namespace Magnefu
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, Scene* scene) :
			m_EntityHandle(entity), m_Scene(scene)
		{}


		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		~Entity();

		template<typename T, typename...Args>
		T& AddComponent(Args&&...args)
		{
			return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

	private:

		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}