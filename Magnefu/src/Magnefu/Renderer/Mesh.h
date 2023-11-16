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
		uint32_t VertexCount;
	};
}