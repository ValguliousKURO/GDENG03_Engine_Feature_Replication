#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	class Rasterizer final : public GraphicsResource
	{
		public:
			Rasterizer(const RasterizerDesc& desc, const GraphicsResourceDesc& gDesc);
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> getRasterizer();

	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizer{};
	};

}




