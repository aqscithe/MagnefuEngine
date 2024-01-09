#pragma once

// -- Graphics ------------------------ //
#include "Magnefu/Graphics/Renderer.hpp"

// -- Core -------------------------- //
#include "Magnefu/Core/glTF.hpp"

#include "Magnefu/Core/Array.h"

// -- vendor Includes --------------- //

#include "cglm/struct/mat3.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/affine.h"

#include "entt.hpp"	


namespace Magnefu
{

	struct Entity;

	struct MeshDraw
	{
		Material* material;

		BufferHandle    index_buffer;
		BufferHandle    position_buffer;
		BufferHandle    tangent_buffer;
		BufferHandle    normal_buffer;
		BufferHandle    texcoord_buffer;
		BufferHandle    material_buffer;

		u32         index_offset;
		u32         position_offset;
		u32         tangent_offset;
		u32         normal_offset;
		u32         texcoord_offset;

		u32         primitive_count;

		// Indices used for bindless textures.
		u16         diffuse_texture_index;
		u16         roughness_texture_index;
		u16         normal_texture_index;
		u16         occlusion_texture_index;

		vec4s       base_color_factor;
		vec4s       metallic_roughness_occlusion_factor;
		vec3s       scale;

		f32         alpha_cutoff;
		u32         flags;
	}; // struct MeshDraw



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

		Array<MeshDraw>						mesh_draws;

		// All graphics resources used by the scene
		Array<Magnefu::TextureResource>		images;
		Array<Magnefu::SamplerResource>		samplers;
		Array<Magnefu::BufferResource>		buffers;

		Magnefu::glTF::glTF                 gltf_scene; // Source gltf file


		entt::registry						registry;
		entt::observer						observer;
		cstring								name;

		// Maybe this should be an array of entity handles, not the entities themselves
		Array<Entity>						entities;

	}; // struct Scene




}