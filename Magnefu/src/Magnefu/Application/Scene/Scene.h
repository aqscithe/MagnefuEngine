#pragma once

// -- vendor Includes --------------- //

#include "entt.hpp"	

#include "Magnefu/Core/glTF.hpp"

#include "Magnefu/Core/Array.h"

namespace Magnefu
{

	//class Entity;



	//class Scene 
	//{
	//	public:
	//		Scene() = default;
	//		Scene(cstring name) { Init(name); }
	//		Scene(const Scene&) = delete;
	//		Scene& operator=(const Scene&) = delete;
	//		~Scene();

	//		static Scene* Create(cstring name);

	//		Entity& CreateEntity(cstring name);

	//		inline entt::registry& GetRegistry() { return m_Registry; }

	//		inline Array<Entity*>& GetEntities() { return m_Entities; }

	//		inline const cstring GetName() const { return m_Name; }


	//		void Init(cstring name);

	//	private:

	//		// -- Entity Listeners -- //
	//		
	//		static void OnCreateEntity(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Entity Created | {}", (uint32_t)entity);
	//		}

	//		static void OnDestroyEntity(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Entity Destroyed | {}", (uint32_t)entity);
	//		}

	//		static void OnUpdateEntity(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Entity Updated | {}", (uint32_t)entity);
	//		}

	//		// -- Component Listeners -- //

	//		static void OnAttachTransformComponent(entt::entity entity)
	//		{ 
	//			MF_CORE_DEBUG("Transform Component Attached | Entity {}", (uint32_t)entity);
	//		}

	//		static void OnAttachMeshComponent(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Mesh Component Attached | Entity {}", (uint32_t)entity);
	//		}

	//		static void OnDetachTransformComponent(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Transform Component Detached | Entity {}", (uint32_t)entity);
	//		}

	//		static void OnDetachMeshComponent(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Mesh Component Detached | Entity {}", (uint32_t)entity);
	//		}

	//		static void OnUpdateTransformComponent(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Transform Component Updated | Entity {}", (uint32_t)entity);
	//		}

	//		static void OnUpdateMeshComponent(entt::entity entity)
	//		{
	//			MF_CORE_DEBUG("Mesh Component Updated | Entity {}", (uint32_t)entity);
	//		}

	//public:


	//	entt::registry m_Registry;
	//	entt::observer m_Observer;
	//	cstring m_Name;
	//	Array<Entity*> m_Entities;
	//};
}