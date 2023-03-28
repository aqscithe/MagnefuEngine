#include "mfpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Magnefu
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}