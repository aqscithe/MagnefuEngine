#pragma once

// -- Graphics Includes ---------------------- //
#include "VulkanCommon.h"

// -- Core Includes -------------------- //
#include "Magnefu/Core/Span.h"


// -- Vendor Includes ------------------ //
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
		LTC1,
		LTC2
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
		FORMAT_R8G8B8A8_SRGB = 43,
		FORMAT_R32G32B32A32_SFLOAT = 109,
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

	struct SamplerInfo
	{
		VkSampler Sampler;
		uint32_t  UseCount;
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
		~Texture();

		inline const VkImageView& GetImageView() const { return m_ImageView; }

		inline const VkSampler& GetSampler() const { return m_TextureSampler; }

	private:
		void CreateTextureImage(const TextureDesc& desc);
		void CreateTextureImageView(const TextureDesc& desc);
		void CreateTextureSampler(const TextureDesc& desc);

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


	private:
		TextureType        m_Type;
		uint32_t           m_MipLevels;
		VkImage            m_Image;
		VkImageView        m_ImageView;
		VkDeviceMemory     m_BufferMemory;
		VkSampler          m_TextureSampler;

		VmaAllocation     m_Allocation;
		VmaAllocationInfo m_AllocInfo;

		static SamplerInfo s_ImageTexSamplerInfo;
		static SamplerInfo s_DataTexSamplerInfo;
	};


	class TextureFactory
	{
	public:
		static Texture* CreateTexture(const TextureDesc& desc);
	};

}
