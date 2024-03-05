#pragma once

// -- Application Includes ---------//


// -- Graphics Includes --------------------- //

// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Matrices.h"



namespace Magnefu
{
//	struct MeshComponent
//	{
//		MeshComponent() = default;
//		MeshComponent(const MeshComponent&) = default;
//		MeshComponent(const MeshDraw& mesh_)
//			: mesh(mesh_) {}
//
//		
//		operator MeshDraw& () { return mesh; }
//		operator const MeshDraw&() const { return mesh; }
//
//		MeshDraw mesh;
//	};
//
//	struct TransformComponent
//	{
//		TransformComponent() = default;
//		TransformComponent(const TransformComponent&) = default;
//		TransformComponent(const mat3s& transform)
//			: Transform(transform) {}
//
//		TransformComponent(const vec3s& pos, const vec3s& rot, const vec3s& scale)
//		{
//			Transform.col[0] = pos;
//			Transform.col[1] = rot;
//			Transform.col[2] = scale;
//		}
//
//		// Direct access functions
//		vec3s& Position() { return Transform.col[0]; }
//		const vec3s& Position() const { return Transform.col[0]; }
//
//		vec3s& Rotation() { return Transform.col[1]; }
//		const vec3s& Rotation() const { return Transform.col[1]; }
//
//		vec3s& Scale() { return Transform.col[2]; }
//		const vec3s& Scale() const { return Transform.col[2]; }
//
//		// Get direct access to Transform member
//		operator mat3s& () { return Transform; }
//		operator const mat3s& () const { return Transform; }
//
//		mat3s Transform;
//	};
}