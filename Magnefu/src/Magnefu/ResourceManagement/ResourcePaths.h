#pragma once
#include <array>

namespace Magnefu
{
	// -- MODELS -- //
	static std::array<const char*, 3> MODEL_PATHS = {
		"res/meshes/corridor.obj",
		"res/meshes/Bronze_shield.obj",
		"res/meshes/Victorian_Painting.obj",

		/*"res/meshes/primitives/3D/planes.obj",
		"res/meshes/area_lights/square_light.obj"*/
	};

	// -- TEXTURES -- //

	struct TexturePaths
	{
		const char* Paths[3];
	};


	static std::array<TexturePaths, 3> TEXTURE_PATHS = {
		
		"res/textures/scificorridor/scene_1001_BaseColor.png",
		"res/textures/scificorridor/scene_1001_ARM.png",
		"res/textures/scificorridor/scene_1001_Normal.png",
			
		"res/textures/Bronze_shield/Bronze_shield_BaseColor.png",
		"res/textures/Bronze_shield/Bronze_shield_ARM.png",
		"res/textures/Bronze_shield/Bronze_shield_Normal.png",
			
		"res/textures/Victorian_Painting/VictorianPaintings_BaseColor_Utility-sRGB-Texture.png",
		"res/textures/Victorian_Painting/VictorianPaintings_ARM_Utility-sRGB-Texture.png",
		"res/textures/Victorian_Painting/VictorianPaintings_Normal_Utility-Raw.png"

		/*"res/textures/surfaces/snow_01_diff_2k.png",
		"res/textures/surfaces/snow_01_arm_2k.png",
		"res/textures/surfaces/snow_01_nor_dx_2k.png",*/
			
	};

	// -- SHADERS -- //
	static const char* SHADER_PATH = "res/shaders/Basic.shader";
	//static const char* SHADER_PATH = "res/shaders/PolygonalLight.shader";
	static const char* PARTICLE_SHADER_PATH = "res/shaders/Particles.shader";
}