#pragma once

// -- Application Includes ---------//


// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/Mesh.h"

// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Matrices.h"



namespace Magnefu
{
	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const Mesh& mesh)
			: Mesh_(mesh) {}

		MeshComponent(void* verticesData, void* indicesData, const uint32_t verticesCount, 
			const uint32_t indicesCount, const uint32_t index)
		{
			Mesh_.VerticesData = verticesData;
			Mesh_.IndicesData = indicesData;
			Mesh_.VerticesCount = verticesCount;
			Mesh_.IndicesCount = indicesCount;
			Mesh_.MeshListIndex = index;

		}

		// TODO: Properly handle the creation of MeshComponent by name
		MeshComponent(std::string& name)
		{
			Mesh_.VerticesData = nullptr;
			Mesh_.IndicesData = nullptr;
			Mesh_.VerticesCount = 0;
			Mesh_.IndicesCount = 0;
			Mesh_.MeshListIndex = 99;
		}

		operator Mesh& () { return Mesh_; }
		operator const Mesh&() const { return Mesh_; }

		Mesh Mesh_;
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Maths::mat3& transform)
			: Transform(transform) {}

		TransformComponent(const Maths::vec3& pos, const Maths::vec3& rot, const Maths::vec3& scale)
		{
			Transform.c[0] = pos;
			Transform.c[1] = rot;
			Transform.c[2] = scale;
		}

		// Direct access functions
		Maths::vec3& Position() { return Transform.position; }
		const Maths::vec3& Position() const { return Transform.position; }

		Maths::vec3& Rotation() { return Transform.rotation; }
		const Maths::vec3& Rotation() const { return Transform.rotation; }

		Maths::vec3& Scale() { return Transform.scale; }
		const Maths::vec3& Scale() const { return Transform.scale; }

		// Get direct access to Transform member
		operator Maths::mat3& () { return Transform; }
		operator const Maths::mat3& () const { return Transform; }

		Maths::mat3 Transform;
	};
}