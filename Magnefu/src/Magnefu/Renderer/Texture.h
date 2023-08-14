#pragma once

#include "Magnefu/Core/Span.h"

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"


namespace Magnefu
{
	enum TextureType
	{
		NONE = -1,
		DIFFUSE,
		ARM,
		NORMAL,
		EMISSIVE,
		DISPLACEMENT,
		MATRIX
	};

	enum TextureTiling
	{
		IMAGE_TILING_NONE = -1,
		IMAGE_TILING_OPTIMAL = 0,
		IMAGE_TILING_LINEAR = 1
	};

	enum TextureFormat
	{
		FORMAT_NONE = -1,
		FORMAT_R8G8B8A8_UNORM = 37,
		FORMAT_R8G8B8A8_SRGB = 43
	};

	enum TextureChannels
	{
		CHANNELS_DEFAULT = 0,
		CHANNELS_GREY = 1,
		CHANNELS_GREY_ALPHA = 2,
		CHANNELS_RGB = 3,
		CHANNELS_RGB_ALPHA = 4
	};

	struct TextureDimensions
	{
		int Width;
		int Height;
		int Channels;
	};

	struct TextureDataBlock
	{
		DataBlock Pixels;
		TextureDimensions Dimensions;
		//TextureChannels RequestedChannels;
	};

	struct TextureDesc
	{
		const char*         DebugName = nullptr;
		uint32_t            Index;
		TextureType         Type = TextureType::NONE;
		TextureTiling       Tiling = TextureTiling::IMAGE_TILING_NONE;
		TextureFormat       Format = TextureFormat::FORMAT_NONE;
	};


	// -- Pre-defined Texture Descriptions -- //
	const TextureDesc DiffuseTextureDesc = {
		"DiffuseTexture",
		0,
		TextureType::DIFFUSE,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_SRGB,
		//TextureChannels::CHANNELS_RGB_ALPHA,
	};

	const TextureDesc ARMTextureDesc = {
		"ARMTexture",
		0,
		TextureType::ARM,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_UNORM,
		//TextureChannels::CHANNELS_RGB_ALPHA,
	};

	const TextureDesc NormalTextureDesc = {
		"NormalTexture",
		0,
		TextureType::NORMAL,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_UNORM,
		//TextureChannels::CHANNELS_RGB_ALPHA
	};


	class Texture
	{
	public:
		Texture(const TextureDesc& desc);
		virtual ~Texture() = default;
	};


	class TextureFactory
	{
	public:
		static Texture* CreateTexture(const TextureDesc& desc);
	};

}
