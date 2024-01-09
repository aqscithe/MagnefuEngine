#pragma once


// -- Application Includes ---------//
#include "Scene.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



// -- vendor Includes ------------------------- //
#include "entt.hpp"



namespace Magnefu
{
	
	struct Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, Scene* scene, cstring name) :
			entity_handle(entity), scene(scene), name(name)
		{}

		~Entity();


		void* GetTransformComponent();
		void* GetMeshComponent();

		template<typename T>
		bool Contains()
		{
			return scene->registry.all_of<T>(entity_handle);
		}
		

		template<typename T, typename...Args>
		T& AddComponent(Args&&...args)
		{
			return scene->registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template<typename T, typename...Args>
		T& ReplaceComponent(Args&&...args)
		{
			return scene->registry.emplace_or_replace<T>(entity_handle, std::forward<Args>(args)...);
		}

		/*template<typename T, typename...Args>
		T& TryAddComponent(Args&&...args)
		{
			return m_Scene->registry.try_emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}*/

	public:
		cstring name;
		entt::entity entity_handle;
		Scene* scene;
	};
}