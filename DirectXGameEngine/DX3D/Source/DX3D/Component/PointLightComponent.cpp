#include <DX3D/Component/PointLightComponent.h>

dx3d::PointLightComponent::PointLightComponent(const ComponentDesc& desc) : Component(desc)
{
}

void dx3d::PointLightComponent::setColor(const Vec3& color) noexcept
{
	m_color = color;
}

dx3d::Vec3 dx3d::PointLightComponent::getColor() const noexcept
{
	return m_color;
}

void dx3d::PointLightComponent::setIntensity(f32 intensity) noexcept
{
	if (intensity < 0.0f) return;
	m_intensity = intensity;
}

dx3d::f32 dx3d::PointLightComponent::getIntensity() const noexcept
{
	return m_intensity;
}

void dx3d::PointLightComponent::setRange(f32 range) noexcept
{
	if (range <= 0.0f) return;
	m_range = range;
}

dx3d::f32 dx3d::PointLightComponent::getRange() const noexcept
{
	return m_range;
}
