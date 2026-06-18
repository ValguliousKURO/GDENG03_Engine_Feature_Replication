#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Game/World.h>

dx3d::MeshComponent::MeshComponent(const ComponentDesc& data)
	: Component(data), m_mesh(nullptr)
{
}

void dx3d::MeshComponent::setMesh(const RefPtr<Mesh>& mesh) noexcept
{
	m_mesh = mesh;
}

const dx3d::RefPtr<dx3d::Mesh>& dx3d::MeshComponent::getMesh() const noexcept
{
	return m_mesh;
}