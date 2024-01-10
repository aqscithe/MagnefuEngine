// -- PCH -- //
#include "mfpch.h"

// -- .h -- //
#include "Scene.h"

// -- Application Includes ---------//
#include "Components.h"
#include "Entity.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //



// -- other Includes ----------------- //
#include <array>




namespace Magnefu
{

}





namespace Magnefu
{

	void Scene::Init(cstring name)
	{

		MF_CORE_INFO("Scene {} Initializing...", name);

		// -- Connect Entity Listeners -- //

		registry.on_construct<entt::entity>().connect<&Scene::OnCreateEntity>();
		registry.on_destroy<entt::entity>().connect<&Scene::OnDestroyEntity>();
		registry.on_update<entt::entity>().connect<&Scene::OnUpdateEntity>();

		// ---------------------------------- //

		// -- Connect Component Listeners -- //

		// Transform
		registry.on_construct<TransformComponent>().connect<&Scene::OnAttachTransformComponent>();
		registry.on_destroy<TransformComponent>().connect<&Scene::OnDetachTransformComponent>();
		registry.on_update<TransformComponent>().connect<&Scene::OnUpdateTransformComponent>();

		// Mesh
		registry.on_construct<MeshComponent>().connect<&Scene::OnAttachMeshComponent>();
		registry.on_destroy<MeshComponent>().connect<&Scene::OnDetachMeshComponent>();
		registry.on_update<MeshComponent>().connect<&Scene::OnUpdateMeshComponent>();


	}

	void Scene::Shutdown()
	{
		//mesh_draws.shutdown();

		registry.clear();

		// -- Disconnect Listeners -- //

		// Entity Listeners
		registry.on_construct<entt::entity>().disconnect();
		registry.on_destroy<entt::entity>().disconnect();
		registry.on_update<entt::entity>().disconnect();

		// Component Listeners
		registry.on_construct<TransformComponent>().disconnect();
		registry.on_construct<MeshComponent>().disconnect();

		registry.on_destroy<TransformComponent>().disconnect();
		registry.on_destroy<MeshComponent>().disconnect();


		registry.on_update<TransformComponent>().disconnect();
		registry.on_update<MeshComponent>().disconnect();

		// --------------------------- //

		entities.shutdown();
	}

	Entity& Scene::CreateEntity(cstring name)
	{
		entities.push(Entity(registry.create(), this, name));
		return entities.back();
	}

}