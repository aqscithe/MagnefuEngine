#pragma once


#define ASSERT(x) if(!(x)) __debugbreak();


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
		yaw = 0.0f;
		pitch = 0.f;
	}
	static Globals s_Instance;
};