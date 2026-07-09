#pragma once
#include <DX3D/Resource/Resource.h>
#include <vector>
#include <span>

namespace dx3d
{

	class MaterialResource final : public Resource
	{
	public:
		explicit MaterialResource(const MaterialResourceDesc& desc);
		MaterialResource(const MaterialResource& material, const MaterialResourceDesc& desc);

		const GraphicsPipelineState& getGraphicsPipelineState() const noexcept;
		void setData(const std::span<const std::byte>& data);
		const std::span<const std::byte> getData() const noexcept;
	public:
		static constexpr std::size_t MaxDataSize{ 256 };
	private:
		GraphicsDevice& m_graphicsDevice;

		RefPtr<VertexShaderSignature> m_vsSignature{};
		RefPtr<ShaderBinary> m_psBinary{};
		RefPtr<GraphicsPipelineState> m_pipeline{};

		std::byte m_data[MaxDataSize]{};
		size_t m_dataSize{};
	};

}