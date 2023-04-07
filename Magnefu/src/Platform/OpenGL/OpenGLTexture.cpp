#include "mfpch.h"

#include "OpenGLTexture.h"
#include "Magnefu/Application.h"

#include <GLAD/glad.h>
#include "stb_image/stb_image.h"

#include "imgui.h"


namespace Magnefu
{
	/*OpenGLTexture::OpenGLTexture(const String& filepath, bool async)
		: m_Filepath(filepath)
	{
		m_HasGenerated = false;
		m_Bound = false;

		m_Slot = Application::Get().GetResourceCache().size<Texture>();

		LoadTexture();

		if (async) return;

		GenerateTexture();

	}*/

	OpenGLTexture::OpenGLTexture(const std::string& filepath)
		: m_Filepath(filepath)
	{
		m_HasGenerated = false;
		m_Bound = false;

		m_Slot = Application::Get().GetResourceCache().size<OpenGLTexture>();

		LoadTexture();
		GenerateTexture();

	}

	OpenGLTexture::~OpenGLTexture()
	{
		MF_CORE_INFO("TEXTURE -- {0} deleted", m_RendererID);
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture::GenerateTexture()
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		SetTextureOptions();

		GenerateTexImage();

		glBindTexture(GL_TEXTURE_2D, 0);

		m_HasGenerated = true;
	}

	void OpenGLTexture::OnImGuiRender() const
	{
		bool test = true;
		ImGui::Image((void*)(intptr_t)m_RendererID, ImVec2(96, 96));
		ImGui::SameLine();
		if (ImGui::Checkbox("Enable", &test))
		{

		}
		ImGui::Text("ID: %d", m_RendererID);
		ImGui::Text("Slot: %d", m_Slot);
		ImGui::Text("File: %s", m_Filepath.c_str());
		ImGui::Text("Size: %dx%d", m_Width, m_Height);
		ImGui::Text("Bits Per Pixel: %d", m_BPP);
		ImGui::Separator();
	}

	void OpenGLTexture::Bind()
	{
		if (m_Bound) return;

		glActiveTexture(GL_TEXTURE0 + m_Slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		m_Bound = true;
	}

	void OpenGLTexture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		m_Bound = false;
	}

	void OpenGLTexture::SetTextureOptions()
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Texture Sampling Technique
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
		//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
		//
		//value = (value > max_anisotropy) ? max_anisotropy : value;
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
	}

	void OpenGLTexture::GenerateTexImage()
	{
		if (!m_texData)
			MF_CORE_WARN("TEXTURE -- No valid texture data to generate texture image.");
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texData);
			glGenerateMipmap(GL_TEXTURE_2D);
		}


		if (m_texData)
			stbi_image_free(m_texData);
	}


	void OpenGLTexture::LoadTexture()
	{
		stbi_set_flip_vertically_on_load(1);
		m_texData = stbi_load(m_Filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);
		if (!m_texData)
			MF_CORE_WARN("TEXTURE -- Texture at '{}' failed to load.", m_Filepath);
		else
			MF_CORE_INFO("TEXTURE -- Loaded texture with width {0}, height {1} and channels {2}", m_Width, m_Height, m_BPP);
	}
}


