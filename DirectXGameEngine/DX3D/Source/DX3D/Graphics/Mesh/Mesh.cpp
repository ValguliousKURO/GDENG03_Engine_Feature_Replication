#include <DX3D/Graphics/Mesh/Mesh.h>

dx3d::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<ui32>& indices)
	: m_vertices(vertices), m_indices(indices)
{
}

const dx3d::Vertex* dx3d::Mesh::getVertices() const noexcept
{
	return m_vertices.data();
}

dx3d::ui32 dx3d::Mesh::getVertexCount() const noexcept
{
	return static_cast<ui32>(m_vertices.size());
}

const dx3d::ui32* dx3d::Mesh::getIndices() const noexcept
{
	return m_indices.data();
}

dx3d::ui32 dx3d::Mesh::getIndexCount() const noexcept
{
	return static_cast<ui32>(m_indices.size());
}
