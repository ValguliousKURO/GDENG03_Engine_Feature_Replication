#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <d3dcompiler.h>

namespace dx3d
{
	class GraphicsPipelineLayout final : public GraphicsResource
	{
	public:
		GraphicsPipelineLayout(const GraphicsPipelineLayoutDesc& desc, const GraphicsResourceDesc& gDesc);
		BinaryData getVSBinaryData() const noexcept;
		BinaryData getPSBinaryData() const noexcept;
		BinaryData getInputElementsData() const noexcept;
		ui32 getMaxTextureSlots() const noexcept;
		ui32 getMaxSamplerSlots() const noexcept;
		ui32 getMaxConstantBufferSlots() const noexcept;

	private:
		void processShaderBinary(ShaderBinary& binary);
	private:
		RefPtr<ShaderBinary> m_vsBinary{};
		RefPtr<ShaderBinary> m_psBinary{};
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_reflections[2]{};

		D3D11_INPUT_ELEMENT_DESC m_elements[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
		ui32 m_numElements{};
		ui32 m_maxTextureSlots{};
		ui32 m_maxSamplerSlots{};
		ui32 m_maxBufferSlots{};
	};
}
