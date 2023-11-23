#include "mfpch.h"
#include "Entity.h"
#include "Magnefu/Scene/Components.h"



namespace Magnefu
{


	Entity::~Entity()
	{

	}

	void* Entity::GetTransformComponent()
	{
		return m_Scene->GetRegistry().try_get<TransformComponent>(m_EntityHandle);
	}

	void* Entity::GetMeshComponent()
	{
		return m_Scene->GetRegistry().try_get<MeshComponent>(m_EntityHandle);
	}
}