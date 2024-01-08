#pragma once

#include "Magnefu/Graphics/RenderConstants.h"

#include <array>


namespace Magnefu
{

	// -- MODELS -- //

	enum ModelType
	{
		MODEL_DEFAULT = 0,
		MODEL_AREA_LIGHT = 1,
	};

	static const uint32_t SCENE_COUNT = 1;

	// Info should come from a text/binary scene file e.g "Level_1.scn"
	struct ResourceInfo
	{
		const char* ModelPath;
		const char* ShaderPath;
		ModelType   ModelType;
		bool        IsTextured;
		bool        IsInstanced;
		uint32_t    InstanceCount;
		uint32_t    SceneIndex;
	};

	static std::array<ResourceInfo, 2> RESOURCE_PATHS = {
		/*"res/meshes/corridor.obj",
		"res/meshes/Bronze_shield.obj",
		"res/meshes/Victorian_Painting.obj",*/

		{
			//{"res/meshes/primitives/3D/plane.obj", "res/shaders/AreaLightLTC.shader", ModelType::MODEL_DEFAULT, true, false, 0},
			{"res/meshes/corridor.obj", "res/shaders/AreaLightLTC.shader", ModelType::MODEL_DEFAULT, true, false, 0, 0},
			{"res/meshes/area_lights/square_light.obj", "res/shaders/AreaLight.shader", ModelType::MODEL_AREA_LIGHT,false, true, MAX_AREA_LIGHTS, 0},
			
		}
	};

	// -- TEXTURES -- //

	struct TexturePaths
	{
		int ModelIndex;
		const char* Paths[3];
	};


	static std::array<TexturePaths, 1> TEXTURE_PATHS = {
		
		/*"res/textures/scificorridor/scene_1001_BaseColor.png",
		"res/textures/scificorridor/scene_1001_ARM.png",
		"res/textures/scificorridor/scene_1001_Normal.png",
			
		"res/textures/Bronze_shield/Bronze_shield_BaseColor.png",
		"res/textures/Bronze_shield/Bronze_shield_ARM.png",
		"res/textures/Bronze_shield/Bronze_shield_Normal.png",
			
		"res/textures/Victorian_Painting/VictorianPaintings_BaseColor_Utility-sRGB-Texture.png",
		"res/textures/Victorian_Painting/VictorianPaintings_ARM_Utility-sRGB-Texture.png",
		"res/textures/Victorian_Painting/VictorianPaintings_Normal_Utility-Raw.png"*/

		0,
		/*"res/textures/surfaces/snow_01/snow_01_diff_2k.png",
		"res/textures/surfaces/snow_01/snow_01_arm_2k.png",
		"res/textures/surfaces/snow_01/snow_01_nor_dx_2k.png",*/

		/*"res/textures/surfaces/metal_plate/metal_plate_diff_2k.png",
		"res/textures/surfaces/metal_plate/metal_plate_arm_2k.png",
		"res/textures/surfaces/metal_plate/metal_plate_nor_dx_2k.png",*/

		/*"res/textures/surfaces/plank_flooring/plank_flooring_diff_2k.png",
		"res/textures/surfaces/plank_flooring/plank_flooring_arm_2k.png",
		"res/textures/surfaces/plank_flooring/plank_flooring_nor_dx_2k.png",*/	

		/*"res/textures/surfaces/painted_metal_shutter/painted_metal_shutter_diff_2k.png",
		"res/textures/surfaces/painted_metal_shutter/painted_metal_shutter_arm_2k.png",
		"res/textures/surfaces/painted_metal_shutter/painted_metal_shutter_nor_dx_2k.png",*/

		/*"res/textures/surfaces/slate_floor/slate_floor_diff_2k.png",
		"res/textures/surfaces/slate_floor/slate_floor_arm_2k.png",
		"res/textures/surfaces/slate_floor/slate_floor_nor_dx_2k.png",*/

		/*"res/textures/surfaces/painted_concrete/painted_concrete_diff_2k.png",
		"res/textures/surfaces/painted_concrete/painted_concrete_arm_2k.png",
		"res/textures/surfaces/painted_concrete/painted_concrete_nor_dx_2k.png",*/

		/*"res/textures/surfaces/aerial_rocks_02/aerial_rocks_02_diff_2k.png",
		"res/textures/surfaces/aerial_rocks_02/aerial_rocks_02_arm_2k.png",
		"res/textures/surfaces/aerial_rocks_02/aerial_rocks_02_nor_dx_2k.png",*/

		"res/textures/scificorridor/scene_1001_BaseColor.png",
		"res/textures/scificorridor/scene_1001_ARM.png",
		"res/textures/scificorridor/scene_1001_Normal.png",
			
	};

	// -- SHADERS -- //
	static const char* PARTICLE_SHADER_PATH = "res/shaders/Particles.shader";
}