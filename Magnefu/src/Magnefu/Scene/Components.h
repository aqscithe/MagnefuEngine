#pragma once

#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Matrices.h"
#include "Magnefu/Renderer/Mesh.h"

namespace Magnefu
{
	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const Mesh& mesh)
			: Mesh_(mesh) {}

		operator Mesh& () { return Mesh_; }
		operator const Mesh&() const { return Mesh_; }

		Mesh Mesh_;
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