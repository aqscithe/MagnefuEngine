#include "mfpch.h"
#include "Camera.h"
#include "SceneCamera.h"


namespace Magnefu
{
	Camera* Camera::Create(const CameraOrientation& orientation, const CameraProps& props)
	{
		return new SceneCamera(orientation, props);
	}
}

