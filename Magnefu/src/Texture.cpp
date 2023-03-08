#include "Texture.h"
#include <GL/glew.h>
#include <iostream>
#include "stb_image/stb_image.h"
#include "Magnefu/Log.h"


Texture::Texture(const String& filepath, bool async)
	: m_filepath(filepath)
{
	m_HasGenerated = false;

	LoadTexture();

	if (async) return;

	GenerateTexture();
	
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture::GenerateTexture()
{
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	SetTextureOptions();

	GenerateTexImage();

	glBindTexture(GL_TEXTURE_2D, 0);

	m_HasGenerated = true;
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetTextureOptions()
{
	// Texture Wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Texture Sampling Technique
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
	//
	//value = (value > max_anisotropy) ? max_anisotropy : value;
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
}

void Texture::GenerateTexImage()
{
	if (!m_texData)
		MF_CORE_WARN("TEXTURE -- No valid texture data to generate texture image.");
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
		

	if(m_texData)
		stbi_image_free(m_texData);
}

// should i do this in the constructor instead of a separate function?
// depends on if i would ever want to overwrite a preexisting texture or just create a new one
void Texture::LoadTexture()
{
	stbi_set_flip_vertically_on_load(1);
	m_texData = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_BPP, 4);
	if (!m_texData)
		MF_CORE_WARN("TEXTURE -- Texture at '{}' failed to load.", m_filepath);
	else
		MF_CORE_INFO("TEXTURE -- Loaded texture with width {0}, height {1} and channels {2}", m_width, m_height, m_BPP);
}
