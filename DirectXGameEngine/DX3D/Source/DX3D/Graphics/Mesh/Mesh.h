#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>
#include <vector>

namespace dx3d
{
	struct Vertex
	{
		Vec3 position;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<ui32>& indices);

		const Vertex* getVertices() const noexcept;
		ui32 getVertexCount() const noexcept;

		const ui32* getIndices() const noexcept;
		ui32 getIndexCount() const noexcept;

	private:
		std::vector<Vertex> m_vertices;
		std::vector<ui32> m_indices;
	};
}
