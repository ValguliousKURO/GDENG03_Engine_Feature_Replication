#pragma once
#include <DX3D/Game/Component.h>
#include <DX3D/Math/Vec3.h>

namespace dx3d
{
	class PointLightComponent final : public Component
	{
		dx3d_typeid(PointLightComponent)
	public:
		explicit PointLightComponent(const ComponentDesc& desc);

		void setColor(const Vec3& color) noexcept;
		Vec3 getColor() const noexcept;

		void setIntensity(f32 intensity) noexcept;
		f32 getIntensity() const noexcept;

		void setRange(f32 range) noexcept;
		f32 getRange() const noexcept;

	private:
		Vec3 m_color{ 1.0f, 0.95f, 0.85f };
		f32 m_intensity{ 1.0f };
		f32 m_range{ 8.0f };
	};
}
