#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Graphics/Mesh/Mesh.h>
#include <DX3D/Graphics/GraphicsDevice.h>


namespace dx3d
{
	class MeshComponent final : public Component
	{
		dx3d_typeid(MeshComponent)
	public:
		explicit MeshComponent(const ComponentDesc& data);

		void setMaterial(const RefPtr<MaterialResource>& material);
		MaterialResource* getMaterial();

		void setMesh(const RefPtr<Mesh>& mesh) noexcept;
		const RefPtr<Mesh>& getMesh() const noexcept;

		RefPtr<VertexBuffer> getOrCreateVertexBuffer(GraphicsDevice& _gDevice);
		RefPtr<IndexBuffer> getOrCreateIndexBuffer(GraphicsDevice& _gDevice);

	private:
		RefPtr<MaterialResource> m_material{};

		RefPtr<Mesh> m_mesh;

		RefPtr<VertexBuffer> m_vertexBuffer;
		RefPtr<IndexBuffer> m_indexBuffer;
	};
}