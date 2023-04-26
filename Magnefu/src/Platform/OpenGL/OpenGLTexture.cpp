#include "mfpch.h"

#include "OpenGLTexture.h"
#include "Magnefu/Application.h"

#include <GLAD/glad.h>
#include "stb_image/stb_image.h"

#include "imgui.h"


namespace Magnefu
{
	OpenGLTexture::OpenGLTexture(const TextureOptions& options, const std::string& filepath)
		: m_Options(options), m_Filepath(filepath)
	{
		MF_PROFILE_FUNCTION();
		m_Bound = false;

		m_Slot = Application::Get().GetResourceCache().size<OpenGLTexture>();

		LoadTexture();
	}

	OpenGLTexture::~OpenGLTexture()
	{
		MF_CORE_INFO("TEXTURE -- {0} deleted", m_RendererID);
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture::LoadTexture()
	{
		MF_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(1);

		if (m_Options & TextureOptions_Skybox)
		{
			// Generate & Bind Texture
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

			std::string skybox[6] = {
				"res/textures/skybox/default/right-px.png",
				"res/textures/skybox/default/left-nx.png",
				"res/textures/skybox/default/top-py.png",
				"res/textures/skybox/default/bottom-ny.png",
				"res/textures/skybox/default/back-nz.png",
				"res/textures/skybox/default/front-pz.png",
			};

			int skyboxCount = sizeof(skybox) / sizeof(std::string);
			{
				MF_PROFILE_SCOPE("Loading Cube Maps");
				for (int i = 0; i < skyboxCount; i++)
				{
					m_texData = stbi_load(skybox[i].c_str(), &m_Width, &m_Height, &m_BPP, 4);
					if (!m_texData)
					{
						MF_CORE_WARN("TEXTURE -- Cube Map at '{}' failed to load.", skybox[i]);
					}
					else
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_texData);
						stbi_image_free(m_texData);

						MF_CORE_INFO("TEXTURE -- Loaded cube map texture with width {0}, height {1} and channels {2}", m_Width, m_Height, m_BPP);
					}
					
				}
			}

			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			// Set Texture Options
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		}
		else
		{
			// Load Texture
			m_texData = stbi_load(m_Filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);
			if (!m_texData)
				MF_CORE_WARN("TEXTURE -- Texture at '{}' failed to load.", m_Filepath);
			else
				MF_CORE_INFO("TEXTURE -- Loaded texture with width {0}, height {1} and channels {2}", m_Width, m_Height, m_BPP);

			// Generate & Bind Texture
			glGenTextures(1, &m_RendererID);
			glBindTexture(GL_TEXTURE_2D, m_RendererID);

			// Generate Texture Image
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texData);
			glGenerateMipmap(GL_TEXTURE_2D);

			// Set Texture Options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
			//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
			//
			//value = (value > max_anisotropy) ? max_anisotropy : value;
			//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

			// Delete Texture Data
			if (m_texData)
				stbi_image_free(m_texData);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}

	void OpenGLTexture::Bind()
	{
		MF_PROFILE_FUNCTION();

		if (m_Bound) return;

		glActiveTexture(GL_TEXTURE0 + m_Slot);

		if (m_Options & TextureOptions_Skybox)
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		else
			glBindTexture(GL_TEXTURE_2D, m_RendererID);

		m_Bound = true;
	}

	void OpenGLTexture::Unbind()
	{
		MF_PROFILE_FUNCTION();

		if (m_Options & TextureOptions_Skybox)
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		else
			glBindTexture(GL_TEXTURE_2D, 0);
		m_Bound = false;
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

	

	/// <summary>
	/// 
	/// </summary>
	
	void OpenGLTexture::GenerateTexture()
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		SetTextureOptions();

		GenerateTexImage();

		glBindTexture(GL_TEXTURE_2D, 0);
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
		{
			MF_PROFILE_SCOPE("Generate Tex Image & Mip Maps");
			if (!m_texData)
				MF_CORE_WARN("TEXTURE -- No valid texture data to generate texture image.");
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texData);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}


		{
			MF_PROFILE_SCOPE("Free Texture Data");
			if (m_texData)
				stbi_image_free(m_texData);
		}
	}

	void OpenGLTexture::CreateTexture()
	{
		{
			MF_PROFILE_SCOPE("Load Texture");
			LoadTexture();
		}

		{
			MF_PROFILE_SCOPE("Generate Texture");
			GenerateTexture();
		}
	}
}


