#pragma once

// -- vendor Includes ------------------------- //
#include "entt.hpp"



namespace Magnefu
{
	
	struct Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, entt::entity parent, u32 child_count, entt::registry* registry, cstring name) :
			entity_handle(entity), parent_handle(parent), child_count(child_count), registry(registry), name(name)
		{}

		~Entity();


		/*void* GetTransformComponent();
		void* GetMeshComponent();*/

		template<typename T>
		bool Contains()
		{
			return registry->all_of<T>(entity_handle);
		}
		

		template<typename T, typename...Args>
		T& AddComponent(Args&&...args)
		{
			return registry->emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template<typename T, typename...Args>
		T& ReplaceComponent(Args&&...args)
		{
			return registry->emplace_or_replace<T>(entity_handle, std::forward<Args>(args)...);
		}

		/*template<typename T, typename...Args>
		T& TryAddComponent(Args&&...args)
		{
			return m_Scene->registry.try_emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}*/

	public:
		cstring name;
		entt::entity entity_handle;
		entt::entity parent_handle;
		entt::registry* registry; 
		u32 child_count;
	};
}