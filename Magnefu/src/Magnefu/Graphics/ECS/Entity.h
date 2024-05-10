#pragma once



// -- vendor Includes ------------------------- //
#include "entt.hpp"

#include <stack>

namespace Magnefu
{

	typedef u32 EntityHandle;

	// While technically components, I will not add component to the names of
	// Parent and Children
	struct Parent 
	{
		entt::entity parent;
	};

	struct Children
	{
		Array<entt::entity> children;
	};

	
	struct Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, entt::registry* registry, cstring name) :
			id(entity), registry(registry), name(name)
		{
		}

		~Entity();


		/*void* GetTransformComponent();
		void* GetMeshComponent();*/

		template<typename T>
		bool Contains()
		{
			return registry->all_of<T>(id);
		}
		

		template<typename T, typename...Args>
		T& AddComponent(Args&&...args)
		{
			return registry->emplace_or_replace<T>(id, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			return registry->get<T>(id);
		}

		template<typename T>
		bool HasComponent()
		{
			return registry->all_of<T>(id);
		}

		/*template<typename T, typename...Args>
		T& TryAddComponent(Args&&...args)
		{
			return m_Scene->registry.try_emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}*/

	public:
		cstring name;
		entt::entity id;
		entt::registry* registry; 
	};



	class EntityManager 
	{
	public:
		void                        init(Allocator* resident_allocator_);
		void						shutdown();

		// Modify this function to also add parent component
		EntityHandle				create_entity(cstring name);
		void						delete_entity(EntityHandle handle);
		Entity& get_entity(EntityHandle handle);
		
		
	private:
		void						remove_child(entt::entity parent, entt::entity child);
		void						add_child(entt::entity parent, entt::entity child);
		void						change_parent(entt::entity parent, entt::entity child);

	public:
		void						remove_child(EntityHandle parent, EntityHandle child);
		void						add_child(EntityHandle parent, EntityHandle child);
		void						change_parent(EntityHandle child, EntityHandle newParent);

		/*template<typename T, typename By>
		bool SortEntitiesByComponent()
		{
			registry.sort<T, By>();
		}*/

		// -- Entity Listeners -- //

		static void             on_create_entity(entt::entity entity);
		static void             on_destroy_entity(entt::entity entity);
		static void             on_update_entity(entt::entity entity);

		// -- Component Listeners -- //

		static void             on_attach_parent_component(entt::entity entity);
		static void             on_attach_children_component(entt::entity entity);
		static void             on_detach_parent_component(entt::entity entity);
		static void             on_detach_children_component(entt::entity entity);
		static void             on_update_parent_component(entt::entity entity);
		static void             on_update_children_component(entt::entity entity);

		static void             on_attach_transform_component(entt::entity entity);
		static void             on_attach_mesh_component(entt::entity entity);
		static void             on_detach_transform_component(entt::entity entity);
		static void             on_detach_mesh_component(entt::entity entity);
		static void             on_update_transform_component(entt::entity entity);
		static void             on_update_mesh_component(entt::entity entity);


	private:
		Array<u32> sparse; // Sparse array for quick access and existence check
		Array<Entity> dense; // Dense array for packed data storage
		Array<u32> handleToDense; // Indirection from handles to dense array
		Array<u32> denseToHandle; // Reverse indirection from dense to handles
		std::stack<EntityHandle> freeHandles; // TODO: (leon) create custom stack structure for more control over memory

		entt::registry          registry;
		entt::observer			observer;
	};

}