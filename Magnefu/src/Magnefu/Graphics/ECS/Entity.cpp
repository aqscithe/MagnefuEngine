// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "Entity.h"

// -- Application Includes ---------//
#include "Components.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



namespace Magnefu
{


	Entity::~Entity()
	{

	}

	/*void* Entity::GetTransformComponent()
	{
		return scene->registry.try_get<TransformComponent>(entity_handle);
	}

	void* Entity::GetMeshComponent()
	{
		return scene->registry.try_get<MeshComponent>(entity_handle);
	}*/


	// -- EntityManager


	void EntityManager::init(Allocator* resident_allocator_)
	{
		sparse.init(resident_allocator_, 16);
		dense.init(resident_allocator_, 16);
		denseToHandle.init(resident_allocator_, 16);
		handleToDense.init(resident_allocator_, 16);


		// -- Connect Entity Listeners -- //

		registry.on_construct<entt::entity>().connect<&EntityManager::on_create_entity>();
		registry.on_destroy<entt::entity>().connect<&EntityManager::on_destroy_entity>();
		registry.on_update<entt::entity>().connect<&EntityManager::on_update_entity>();

		// ---------------------------------- //

		// -- Connect Component Listeners -- //

		// Transform
		//registry.on_construct<TransformComponent>().connect<&Scene::OnAttachTransformComponent>();
		//registry.on_destroy<TransformComponent>().connect<&Scene::OnDetachTransformComponent>();
		//registry.on_update<TransformComponent>().connect<&Scene::OnUpdateTransformComponent>();

		//// Mesh
		//registry.on_construct<MeshComponent>().connect<&Scene::OnAttachMeshComponent>();
		//registry.on_destroy<MeshComponent>().connect<&Scene::OnDetachMeshComponent>();
		//registry.on_update<MeshComponent>().connect<&Scene::OnUpdateMeshComponent>();
	}

	void EntityManager::shutdown()
	{

		registry.clear();

		// -- Disconnect Listeners -- //

		// Entity Listeners
		registry.on_construct<entt::entity>().disconnect();
		registry.on_destroy<entt::entity>().disconnect();
		registry.on_update<entt::entity>().disconnect();

		// Component Listeners
		/*registry.on_construct<TransformComponent>().disconnect();
		registry.on_construct<MeshComponent>().disconnect();

		registry.on_destroy<TransformComponent>().disconnect();
		registry.on_destroy<MeshComponent>().disconnect();


		registry.on_update<TransformComponent>().disconnect();
		registry.on_update<MeshComponent>().disconnect();*/

		// --------------------------- //

		dense.shutdown();
		sparse.shutdown();
		denseToHandle.shutdown();
		handleToDense.shutdown();
	}

	EntityHandle EntityManager::create_entity(cstring name, entt::entity parent, u32 child_count)
	{
		EntityHandle handle;
		if (!freeHandles.empty())
		{
			// Retrieve available handle
			handle = freeHandles.top();
			freeHandles.pop();

			// Ensure that dense is large enough to accomadate handle
			if (handle >= dense.size)
			{
				dense.set_size(dense.size + 1);
			}

			// Create new entity
			Entity entity = Entity(registry.create(), parent, child_count, &registry, name);

			// Assign new entity to correct location in dense
			dense[handle] = entity;
		}
		else
		{
			handle = static_cast<EntityHandle>(handleToDense.size);

			// Create a new entity in the registry
			entt::entity newEntity = registry.create();
			Entity entity = Entity(newEntity, parent, child_count, &registry, name);

			// Add the new entity to the 'dense' array
			dense.push(entity);

			// Map the new handle to the new dense index
			handleToDense.push(dense.size - 1); // handleToDense maps the handle to the index in 'dense'

			// Ensure that the reverse mapping is maintained as well
			denseToHandle.push(handle); // denseToHandle maps the index in 'dense' back to the handle
		}
		return handle;
	}

	void EntityManager::delete_entity(EntityHandle handle)
	{
		// Find entity in dense array via handleToDense
		// Swap with last entity in dense array
		// Update mappings
		// Optional: defer actual deletion or re-use the spot
	}

	Entity& EntityManager::get_entity(EntityHandle handle)
	{
		// Direct access via indirection table
		return dense[handleToDense[handle]];
	}

	// -- Entity Listeners -- //

	void EntityManager::on_create_entity(entt::entity entity)
	{
		MF_CORE_DEBUG("Entity Created | {}", (uint32_t)entity);
	}

	void EntityManager::on_destroy_entity(entt::entity entity)
	{
		MF_CORE_DEBUG("Entity Destroyed | {}", (uint32_t)entity);
	}

	void EntityManager::on_update_entity(entt::entity entity)
	{
		MF_CORE_DEBUG("Entity Updated | {}", (uint32_t)entity);
	}

	// -- Component Listeners -- //

	void EntityManager::on_attach_transform_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Transform Component Attached | Entity {}", (uint32_t)entity);
	}

	void EntityManager::on_attach_mesh_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Mesh Component Attached | Entity {}", (uint32_t)entity);
	}

	void EntityManager::on_detach_transform_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Transform Component Detached | Entity {}", (uint32_t)entity);
	}

	void EntityManager::on_detach_mesh_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Mesh Component Detached | Entity {}", (uint32_t)entity);
	}

	void EntityManager::on_update_transform_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Transform Component Updated | Entity {}", (uint32_t)entity);
	}

	void EntityManager::on_update_mesh_component(entt::entity entity)
	{
		MF_CORE_DEBUG("Mesh Component Updated | Entity {}", (uint32_t)entity);
	}

}