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

dx3d::RefPtr<dx3d::VertexBuffer> dx3d::MeshComponent::getOrCreateVertexBuffer(GraphicsDevice& _gDevice)
{
	if (!m_vertexBuffer)
	{
		m_vertexBuffer = _gDevice.createVertexBuffer({
			m_mesh->getVertices(),
			m_mesh->getVertexCount(),
			sizeof(Vertex)
			});
	}

	return m_vertexBuffer;
}

dx3d::RefPtr<dx3d::IndexBuffer> dx3d::MeshComponent::getOrCreateIndexBuffer(GraphicsDevice& _gDevice)
{
	if (!m_indexBuffer)
	{
		m_indexBuffer = _gDevice.createIndexBuffer({
			m_mesh->getIndices(),
			m_mesh->getIndexCount()
			});
	}

	return m_indexBuffer;
}


// Create temporary buffers (ideally cache these)
			/*auto vb = m_graphicsDevice.createVertexBuffer({
				vertexData, vertexCount, sizeof(Vertex)
				});
			auto ib = m_graphicsDevice.createIndexBuffer({
				indexData, indexCount
				});*/