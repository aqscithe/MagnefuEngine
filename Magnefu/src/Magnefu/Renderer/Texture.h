#pragma once

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
		DISPLACEMENT
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

	enum TextureOptions 
	{
		TextureOptions_None = 0,
		TextureOptions_Skybox = BIT(0),
		TextureOptions_All = 1
	};


	struct TextureDimensions
	{
		int Width;
		int Height;
		int Channels;
	};

	struct TextureDesc
	{
		const char*         DebugName = nullptr;
		TextureType         Type = TextureType::NONE;
		TextureTiling       Tiling = TextureTiling::IMAGE_TILING_NONE;
		TextureFormat       Format = TextureFormat::FORMAT_NONE;
		TextureChannels     RequestedChannels = TextureChannels::CHANNELS_DEFAULT;
		//Span<const uint8_t> InitData;
	};


	// -- Pre-defined Texture Descriptions -- //
	const TextureDesc DiffuseTextureDesc = {
		"DiffuseTexture",
		TextureType::DIFFUSE,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_SRGB,
		TextureChannels::CHANNELS_RGB_ALPHA,
	};

	const TextureDesc ARMTextureDesc = {
		"ARMTexture",
		TextureType::ARM,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_UNORM,
		TextureChannels::CHANNELS_RGB_ALPHA,
	};

	const TextureDesc NormalTextureDesc = {
		"NormalTexture",
		TextureType::NORMAL,
		TextureTiling::IMAGE_TILING_OPTIMAL,
		TextureFormat::FORMAT_R8G8B8A8_UNORM,
		TextureChannels::CHANNELS_RGB_ALPHA
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
