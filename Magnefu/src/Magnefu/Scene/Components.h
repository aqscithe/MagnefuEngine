#pragma once

#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Matrices.h"

namespace Magnefu
{
	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const uint32_t mesh)
			: Mesh(mesh) {}

		operator uint32_t () { return Mesh; }
		operator const uint32_t() const { return Mesh; }

		uint32_t Mesh;
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
}