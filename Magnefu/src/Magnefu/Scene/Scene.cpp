#include "mfpch.h"
#include "Scene.h"
#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Matrices.h"

#include <array>

namespace Magnefu
{
	struct MeshComponent
	{
		MeshComponent() = default;
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Maths::mat4& transform) 
			: Transform(transform) {}

		operator Maths::mat4& () { return Transform; }
		operator const Maths::mat4& () const { return Transform; }

		Maths::mat4 Transform;
	};

	Scene::Scene()
	{
		// Create a single entity
		entt::entity entity = m_Registry.create();

		std::array<entt::entity, 40> entities;

		// Create several entities given a range of iterators
		m_Registry.create(entities.begin(), entities.end());

		MF_CORE_DEBUG("Enitity byte size: {}", sizeof(entities));

		// Attach a component to an entity
		m_Registry.emplace<TransformComponent>(entity);

		// .emplace() returns a reference to the attached component
		auto& transform_ = m_Registry.emplace<TransformComponent>(entities[2], Maths::mat4(1.f));

		// Attach a component to a range of entities
		m_Registry.insert<TransformComponent>(entities.begin() + 20, entities.end() - 7);

		// Get a view of entities that have a specific component(s)
		auto view = m_Registry.view<TransformComponent>();

		// Attaching a mesh component to all entities in the view I created above
		m_Registry.insert<MeshComponent>(view.begin(), view.end());

		
		for (auto& ent : view)
		{
			TransformComponent& transform = view.get<TransformComponent>(ent);
		}

		m_Registry.patch<TransformComponent>(entity, [](auto& trans) {trans.Transform = Maths::identity(); });
		if (m_Registry.all_of<TransformComponent>(entities[4]))
		{
			m_Registry.replace<TransformComponent>(entities[4], Maths::mat4(3.f));
		}
		

		m_Registry.destroy(entity);

		m_Registry.destroy(entities.begin(), entities.end());
		
	}

	Scene::~Scene()
	{
	
	}
}