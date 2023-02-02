#include "Globals.h"
#include <GL/glew.h>
#include <iostream>


Globals Globals::s_Instance;


void Debug::GLClearError()
{
	while (glGetError());
}

bool Debug::GLLogCall(const char* function, const char* file, int line)
{
	if (unsigned int error = glGetError())
	{
		std::cout << "OpenGL error: " << error << std::endl;
		std::cout << file << " " << line << std::endl;
		return false;
	}
	return true;
}