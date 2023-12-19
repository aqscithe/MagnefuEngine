// - PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "Camera.h"

// Application Includes ------------ //
#include "SceneCamera.h"


namespace Magnefu
{
	Camera* Camera::Create(const CameraOrientation& orientation, const CameraProps& props)
	{
		return new SceneCamera(orientation, props);
	}
}

