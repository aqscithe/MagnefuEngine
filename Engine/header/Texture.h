#pragma once

#include "Globals.h"

class Texture
{
private:
	unsigned int m_RendererID;
	String m_filepath;
	unsigned char* m_texData;
	int m_width, m_height, m_BPP;

	bool m_HasGenerated;
	bool m_Bound;

public:
	Texture(const String& filepath, bool async = false);
	~Texture();

	void Bind(unsigned int slot = 0)   const;
	void Unbind() const;

	unsigned int GetRendererID() const { return m_RendererID; };
	bool GetGenerationStatus() const { return m_HasGenerated; };
	String GetFilepath() const { return m_filepath; };

	void GenerateTexture();

private:
	void SetTextureOptions();
	void LoadTexture();
	void GenerateTexImage();
	

};