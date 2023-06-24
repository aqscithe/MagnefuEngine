#pragma once

#include <unordered_map>
#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"


namespace Magnefu
{
	using String = std::string;

	enum class SceneDataType
	{
		Mat4 = 0,
		Vec3,
		Vec4,
		Float,
		Int,
		IntPtr,
		Bool,
		UInt32
	};

	struct SceneData
	{
		SceneData() {}
		// will need something akin to an object id(identifier for
		// object in the scene) in the future
		std::unordered_map<String, Maths::mat4> Mat4;
		std::unordered_map<String, Maths::vec4> Vec4;
		std::unordered_map<String, Maths::vec3> Vec3;
		std::unordered_map<String, float> Float;
		std::unordered_map<String, int> Int;
		std::unordered_map<String, uint32_t> UInt32;
		std::unordered_map<String, int*> IntPtr;
		std::unordered_map<String, bool> Bool;
		

	};

}