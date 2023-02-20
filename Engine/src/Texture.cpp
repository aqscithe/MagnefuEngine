#include "Texture.h"
#include <GL/glew.h>
#include <iostream>
#include "vendor/stb_image/stb_image.h"


Texture::Texture(const String& filepath, bool async)
	: m_filepath(filepath)
{
	LoadTexture();

	if (async) return;

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	SetTextureOptions();

	GenerateTexImage();

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::SetTextureOptions()
{
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
}

void Texture::GenerateTexImage()
{
	if (!m_texData)
		std::cout << "No valid texture data to generate texture image." << std::endl;
	else
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texData));

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
		std::cout << "Texture at '" << m_filepath << "' failed to load." << std::endl;
	else
		std::cout << "Loaded texture with width " << m_width << ", height " << m_height << " and channels " << m_BPP << std::endl;

		
}
