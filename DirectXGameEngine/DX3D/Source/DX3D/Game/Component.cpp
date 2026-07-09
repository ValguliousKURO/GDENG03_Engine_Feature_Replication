#include <DX3D/Game/Component.h>
#include <DX3D/Game/World.h>

dx3d::Component::Component(const ComponentDesc& desc) : Identifiable(desc.base), m_object(desc.object), m_world(desc.world), m_context(desc.context)
{
}

dx3d::GameObject& dx3d::Component::getGameObject() noexcept
{
	return m_object;
}