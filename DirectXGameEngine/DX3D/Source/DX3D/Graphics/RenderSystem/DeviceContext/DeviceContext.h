#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Math/Vec4.h>
#include <span>
#include <array>

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
		void setTextures(const std::span<Texture*>& textures);
		void setSamplers(const std::span<Sampler*>& samplers);
		void updateConstantBuffer(const ConstantBuffer& buffer, const std::span<const std::byte>& data);
		void drawTriangleList(ui32 vertexCount, ui32 startVertexLocation);
		void drawIndexedTriangleList(ui32 indexCount, ui32 startVertexIndex, ui32 startIndexLocation);
	public:
		static constexpr std::size_t MaxConstantBuffersPerStage{ D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT };
		static constexpr std::size_t MaxSamplersPerStage{ D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT };
		static constexpr std::size_t MaxTexturesPerStage{ D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT };
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context{};
		std::array<ID3D11Buffer*, MaxConstantBuffersPerStage> m_constantBuffers{};
		std::array<ID3D11ShaderResourceView*, MaxTexturesPerStage> m_srv{};
		std::array<ID3D11SamplerState*, MaxSamplersPerStage> m_samplers{};

		friend class GraphicsDevice;
	};
}
