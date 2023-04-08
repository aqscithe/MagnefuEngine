#pragma once

#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"

namespace Magnefu
{
	enum class CameraType
	{
		None = 0,
		Orthographic = 1,
		Perspective = 2
	};

	struct CameraProps
	{
		CameraType Type;
		float      AspectRatio;
		float      FOV;
		float      Near;
		float      Far;
		float      Speed;

		CameraProps(
			CameraType type = CameraType::Perspective, 
			float aspect = 1920.f / 1080.f,  // Should come from the window
			float fov = 45.f, 
			float _near = 0.01f, 
			float _far = 100.f,
			float speed = 8.f
		)
			: Type(type), AspectRatio(aspect), FOV(fov), Near(_near), Far(_far), Speed(speed)
		{}
		
	};

	struct CameraOrientation
	{
		Maths::vec3 Position;
		Maths::vec3 Target;
		float       Yaw;
		float       Pitch;

		CameraOrientation(
			const Maths::vec3& position = { 0.f, 0.f, 4.f }, 
			const Maths::vec3& target = { 0.f, 0.f, 0.f }, 
			float yaw = 0.f, 
			float pitch = 0.f
		)
			: Position(position), Target(target), Yaw(yaw), Pitch(pitch)
		{}
	};

	// all camera data that may be needed outside of the camera
		// useful as we can just pass the struct, not the whole class
	struct CameraData
	{
		CameraType  Type;
		Maths::vec3 Position;
		float       Yaw;
		float       Pitch;
		float       AspectRatio;
		float       FOV;
		float       Near;
		float       Far;
		float       Speed;
	};

	class Camera
	{

	public:
		virtual ~Camera() = default;

		virtual const Maths::mat4& GetView() const = 0;
		virtual const Maths::mat4& GetProjection() const = 0;
		virtual const Maths::mat4& GetVP() const = 0;
		virtual CameraData& GetData() = 0;

		virtual Maths::mat4& CalculateView() = 0;
		virtual Maths::mat4& CalculateProjection() = 0;
		virtual Maths::mat4& CalculateVP() = 0;
		virtual void ProcessInput(float deltaTime) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void SetDefaultProps() = 0;

		static Camera* Create(const CameraOrientation& orientation = CameraOrientation(), const CameraProps& props = CameraProps());

	protected:
		virtual void Init(const CameraOrientation& orientation, const CameraProps& props) = 0;
	};

}

