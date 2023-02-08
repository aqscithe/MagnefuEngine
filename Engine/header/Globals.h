#pragma once

#include <string>


#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) Debug::GLClearError();\
		x;\
		ASSERT(Debug::GLLogCall(#x, __FILE__, __LINE__));




typedef std::string String;

class Globals
{
public:
	
	Globals(const Globals&) = delete;

	int WIDTH;
	int HEIGHT;

	float fovY;
	float yaw;
	float pitch;

	static Globals& Get() { return s_Instance; }

private:
	Globals()
	{
		WIDTH = 1920;
		HEIGHT = 1080;
		fovY = 45.f;
		yaw = 0.f;
		pitch = 0.f;
	}
	static Globals s_Instance;
};


namespace Debug
{
	void GLClearError();

	bool GLLogCall(const char* function, const char* file, int line);

}