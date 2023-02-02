#pragma once

#include "Globals.h"

class Texture
{
private:
	unsigned int m_RendererID;
	String m_filepath;
	unsigned char* m_texData;
	int m_width, m_height, m_BPP;

public:
	Texture(const String& filepath);
	~Texture();

	void Bind(unsigned int slot = 0)   const;
	void Unbind() const;


	void SetTextureOptions();

	void LoadTexture();
	void GenerateTexImage();

};