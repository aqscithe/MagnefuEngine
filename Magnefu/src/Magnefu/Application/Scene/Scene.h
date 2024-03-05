#pragma once

// -- Graphics ------------------------ //
#include "Magnefu/Graphics/Renderer.hpp"

// -- Core -------------------------- //
#include "Magnefu/Core/glTF.hpp"

#include "Magnefu/Core/Array.h"

// -- vendor Includes --------------- //

#include "entt.hpp"	
#include "enkiTS/TaskScheduler.h"



namespace Magnefu
{
	struct Entity;
	


	// TODO: Will eventually create a full on class for ECS entt data
// When creating scene manager take inspiration from the service
// manager implementation...
	struct Scene
	{

		void Init(cstring name);
		void Shutdown();

		Entity& CreateEntity(cstring name);

		/*template<typename T, typename By>
		bool SortEntitiesByComponent()
		{
			registry.sort<T, By>();
		}*/



	private:

		// -- Entity Listeners -- //
				
		static void OnCreateEntity(entt::entity entity)
		{
			MF_CORE_DEBUG("Entity Created | {}", (uint32_t)entity);
		}

		static void OnDestroyEntity(entt::entity entity)
		{
			MF_CORE_DEBUG("Entity Destroyed | {}", (uint32_t)entity);
		}

		static void OnUpdateEntity(entt::entity entity)
		{
			MF_CORE_DEBUG("Entity Updated | {}", (uint32_t)entity);
		}

		// -- Component Listeners -- //

		static void OnAttachTransformComponent(entt::entity entity)
		{ 
			MF_CORE_DEBUG("Transform Component Attached | Entity {}", (uint32_t)entity);
		}

		static void OnAttachMeshComponent(entt::entity entity)
		{
			MF_CORE_DEBUG("Mesh Component Attached | Entity {}", (uint32_t)entity);
		}

		static void OnDetachTransformComponent(entt::entity entity)
		{
			MF_CORE_DEBUG("Transform Component Detached | Entity {}", (uint32_t)entity);
		}

		static void OnDetachMeshComponent(entt::entity entity)
		{
			MF_CORE_DEBUG("Mesh Component Detached | Entity {}", (uint32_t)entity);
		}

		static void OnUpdateTransformComponent(entt::entity entity)
		{
			MF_CORE_DEBUG("Transform Component Updated | Entity {}", (uint32_t)entity);
		}

		static void OnUpdateMeshComponent(entt::entity entity)
		{
			MF_CORE_DEBUG("Mesh Component Updated | Entity {}", (uint32_t)entity);
		}



		// -- Members -------------------------------------- //
	public:

		entt::registry						registry;
		entt::observer						observer;
		cstring								name;

		// Maybe this should be an array of entity handles, not the entities themselves
		Array<Entity>						entities;

	}; // struct Scene




}