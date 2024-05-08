#pragma once



// -- vendor Includes ------------------------- //
#include "entt.hpp"

#include <stack>

namespace Magnefu
{

	typedef u32 EntityHandle;
	
	struct Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, entt::entity parent, u32 child_count, entt::registry* registry, cstring name) :
			entity_handle(entity), parent_handle(parent), registry(registry), name(name)
		{
			children.init(&MemoryService::Instance()->systemAllocator, child_count);
		}

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
		u32 entity_index;
		u32 parent_index;
		entt::registry* registry; 
		Array<entt::entity> children;
	};



	class EntityManager 
	{
	public:
		void                        init(Allocator* resident_allocator_);
		void						shutdown();

		EntityHandle				create_entity(cstring name, entt::entity parent, u32 child_count);
		void						delete_entity(EntityHandle handle);
		Entity&						get_entity(EntityHandle handle);

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