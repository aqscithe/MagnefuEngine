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
}