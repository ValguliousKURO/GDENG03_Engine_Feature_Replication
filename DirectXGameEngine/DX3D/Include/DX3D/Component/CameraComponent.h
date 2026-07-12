#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Math/Rect.h>
#include <DX3D/Math/Mat4x4.h>

namespace dx3d
{
	enum class ProjectionMode
	{
		Perspective,
		Orthographic
	};

	class CameraComponent final : public Component
	{
		dx3d_typeid(CameraComponent)
		public:
			explicit CameraComponent(const ComponentDesc& data);

			Mat4x4 getViewMatrix() noexcept;
			Mat4x4 getProjectionMatrix() const noexcept;

			void setFarPlane(f32 farPlane) noexcept;
			f32 getFarPlane() const noexcept;

			void setNearPlane(f32 nearPlane) noexcept;
			f32 getNearPlane()const noexcept;

			void setFieldOfView(f32 fieldOfView) noexcept;
			f32 getFieldOfView() const noexcept;

			void setViewportSize(const Rect& size) noexcept;
			Rect getViewportSize() const noexcept;

			void setProjectionMode(ProjectionMode mode) noexcept;
			ProjectionMode getProjectionMode() const noexcept;

			void setOrthoZoom(f32 zoom) noexcept;
			f32 getOrthoZoom() const noexcept;

		private:
			void computeProjectionMatrix() noexcept;

		private:
			Mat4x4 m_projection{};
			ProjectionMode m_projectionMode = ProjectionMode::Perspective;

			f32 m_nearPlane = 0.01f;
			f32 m_farPlane = 100.0f;
			f32 m_fieldOfView = 1.3f;
			Rect m_viewportSize{ 1,1 };
			f32 m_orthoZoom = 0.01f;

			bool m_dirty{ true };
	};

}