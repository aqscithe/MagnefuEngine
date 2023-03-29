#include "mfpch.h"
#include "Camera.h"
#include "OrthographicCamera.h"
#include "PerspectiveCamera.h"


namespace Magnefu
{
	Camera* Camera::Create(const CameraOrientation& orientation, const CameraProps& props)
	{
		switch (props.Type)
		{
		case CameraType::None:
			MF_CORE_ASSERT(false, "CameraType::None - Unknown camera type");
			return nullptr;
		case CameraType::Orthographic:
			return new OrthographicCamera(orientation, props);
		case CameraType::Perspective:
			return new PerspectiveCamera(orientation, props);
		}

		MF_CORE_ASSERT(false, "Unknown camera type");
		return nullptr;
	}
}

