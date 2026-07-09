#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Math/Vec4.h>
#include <span>

namespace dx3d
{
	class DeviceContext final: public GraphicsResource
	{
	public:
		explicit DeviceContext(const GraphicsResourceDesc& gDesc);
		void clearAndSetBackBuffer(const SwapChain& swapChain, const Vec4& color);
		void setGraphicsPipelineState(const GraphicsPipelineState& pipeline);
		void setVertexBuffer(const VertexBuffer& buffer);
		void setIndexBuffer(const IndexBuffer& buffer);
		void setViewportSize(const Rect& size);
		void setConstantBuffers(const std::span<ConstantBuffer*>& buffers);
		void updateConstantBuffer(const ConstantBuffer& buffer, const std::span<const std::byte>& data);
		void drawTriangleList(ui32 vertexCount, ui32 startVertexLocation);
		void drawIndexedTriangleList(ui32 indexCount, ui32 startVertexIndex, ui32 startIndexLocation);
	public:
		static constexpr std::size_t MaxConstantBuffersPerStage{ 16 };
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context{};
		ID3D11Buffer* m_constantBuffers[MaxConstantBuffersPerStage]{};

		friend class GraphicsDevice;
	};
}
