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

	

	Scene::Scene(std::string& name) :
		m_Name(name)
	{
		// -- Connect Entity Listeners -- //

		m_Registry.on_construct<entt::entity>().connect<&Scene::OnCreateEntity>();
		m_Registry.on_destroy<entt::entity>().connect<&Scene::OnDestroyEntity>();
		m_Registry.on_update<entt::entity>().connect<&Scene::OnUpdateEntity>();

		// ---------------------------------- //

		// -- Connect Component Listeners -- //

		// Transform
		m_Registry.on_construct<TransformComponent>().connect<&Scene::OnAttachTransformComponent>();
		m_Registry.on_destroy<TransformComponent>().connect<&Scene::OnDetachTransformComponent>();
		m_Registry.on_update<TransformComponent>().connect<&Scene::OnUpdateTransformComponent>();
		
		// Mesh
		m_Registry.on_construct<MeshComponent>().connect<&Scene::OnAttachMeshComponent>();
		m_Registry.on_destroy<MeshComponent>().connect<&Scene::OnDetachMeshComponent>();
		m_Registry.on_update<MeshComponent>().connect<&Scene::OnUpdateMeshComponent>();

		MF_CORE_INFO("Scene {} Created", name);
		

		// Another method for connecting listeners
		/*entt::sigh_helper(m_Registry)
			.with<TransformComponent>()
				.on_construct<&Scene::OnAttachTransformComponent>()
				.on_destroy<&Scene::OnDetachTransformComponent>()
				.on_update<&Scene::OnUpdateTransformComponent>()
			.with<MeshComponent>()
				.on_construct<&Scene::OnAttachMeshComponent>()
				.on_destroy<&Scene::OnDetachMeshComponent>()
				.on_update<&Scene::OnUpdateMeshComponent>();*/

		// ---------------------------------- //

		//m_Observer.connect(m_Registry, entt::collector.update<TransformComponent>());
		//m_Observer.connect(m_Registry, entt::collector.group<TransformComponent, MeshComponent>());

		//// Create a single entity
		//entt::entity entity = m_Registry.create();

		//std::array<entt::entity, 40> entities;

		//// Create several entities given a range of iterators
		//m_Registry.create(entities.begin(), entities.end());

		//MF_CORE_DEBUG("Enitity byte size: {}", sizeof(entities));

		//// Attach a component to an entity
		//m_Registry.emplace<TransformComponent>(entity);

		//// .emplace() returns a reference to the attached component
		//auto& transform_ = m_Registry.emplace<TransformComponent>(entities[2], Maths::mat4(1.f));

		//// Attach a component to a range of entities
		//m_Registry.insert<TransformComponent>(entities.begin() + 20, entities.end() - 7);

		//// Get a view of entities that have a specific component(s)
		//auto view = m_Registry.view<TransformComponent>();

		//// Attaching a mesh component to all entities in the view I created above
		//m_Registry.insert<MeshComponent>(view.begin(), view.end());

		//
		//for (auto ent : view)
		//{
		//	TransformComponent& transform = view.get<TransformComponent>(ent);
		//}

		//m_Registry.patch<TransformComponent>(entity, [](auto& trans) {trans.Transform = Maths::identity(); });
		//if (m_Registry.all_of<TransformComponent>(entities[2]))
		//{
		//	m_Registry.replace<TransformComponent>(entities[2], Maths::mat4(3.f));
		//}

		///*for (const auto entity : m_Observer)
		//{
		//	MF_CORE_ASSERT("Observed entity with updated transform component: {}", (uint32_t)entity);
		//}
		//m_Observer.clear();*/

		//// Equivalent to above commented code
		//// This means that the observer is cleared by default when using the each() member function for iteration
		//m_Observer.each([](const auto entity) {
		//	MF_CORE_INFO("Observed entity : {}", (uint32_t)entity);
		//});


		//

		//
		//auto view2 = m_Registry.view<TransformComponent, MeshComponent>();

		//auto group = m_Registry.group<TransformComponent, MeshComponent>();

		//// Iterate using for each on group
		//for (auto ent : group)
		//{
		//	auto [transform, mesh] = group.get<TransformComponent, MeshComponent>(ent);
		//}

		//// Iterate using for each on view
		//for (auto ent : view2)
		//{
		//	auto [transform, mesh] = view2.get<TransformComponent, MeshComponent>(ent);
		//	mesh.Mesh_.VertexCount = 3;
		//	transform = Maths::identity();
		//}

		//// Iterate using callback
		//view2.each([](auto entity, auto& transform, auto& mesh) {
		//	mesh.Mesh_.VertexCount = 5;
		//	transform = Maths::mat4(1.4f);
		//		
		//});
		//
		//for (auto&& [entity, transform, mesh] : view2.each())
		//{

		//}

		//// Remove component from entitiy
		//m_Registry.erase<TransformComponent>(entities[2]);
		//// Safer - checks if component exists first (not sure if less performant)
		//m_Registry.remove<TransformComponent>(entities[2]);

		//m_Registry.clear();

		//// Calling destroy indiscriminantly is not safe. Must check that entities are still valid
		//if (m_Registry.valid(entity))
		//{
		//	m_Registry.destroy(entity);
		//}
		//	
		//
		//// Unsafe destruction
		//// m_Registry.destroy(entities.begin(), entities.end());

		//// Safe destruction
		//for (auto it = entities.begin(); it != entities.end(); it++)
		//{
		//	if (m_Registry.valid(*it))
		//	{
		//		m_Registry.destroy(*it);
		//	}
		//}
		
	}

	Scene::~Scene()
	{
		m_Registry.clear();

		// -- Disconnect Listeners -- //

		// Entity Listeners
		m_Registry.on_construct<entt::entity>().disconnect();
		m_Registry.on_destroy<entt::entity>().disconnect();
		m_Registry.on_update<entt::entity>().disconnect();

		// Component Listeners
		m_Registry.on_construct<TransformComponent>().disconnect();
		m_Registry.on_construct<MeshComponent>().disconnect();

		m_Registry.on_destroy<TransformComponent>().disconnect();
		m_Registry.on_destroy<MeshComponent>().disconnect();

		
		m_Registry.on_update<TransformComponent>().disconnect();
		m_Registry.on_update<MeshComponent>().disconnect();

		// --------------------------- //
	}

	

	Scene* Scene::Create(std::string& name)
	{
		return new Scene(name);
	}

	Entity& Scene::CreateEntity(std::string& name)
	{

		m_Entities.emplace_back(new Entity(m_Registry.create(), this, name));

		return *m_Entities.back();
	}

}