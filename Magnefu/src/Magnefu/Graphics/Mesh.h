#pragma once

#include "Magnefu/Core/Maths/Vectors.h"

namespace Magnefu
{
	struct Vertex
	{
		Maths::vec3 pos;
		Maths::vec3 color;
		Maths::vec3 normal;
		Maths::vec3 tangent;
		Maths::vec3 bitangent;
		Maths::vec2 texCoord;

	};

	struct Mesh
	{
		
		// TODO: a lot of the SceneObject functionality needs to be moved

		void* VerticesData;
		void* IndicesData;
		uint32_t VerticesCount;
		uint32_t IndicesCount;
		uint32_t MeshListIndex;
	};
}