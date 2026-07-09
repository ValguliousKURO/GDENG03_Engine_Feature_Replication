#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	class Sampler final : public GraphicsResource
	{
	public:
		Sampler(const SamplerDesc& desc, const GraphicsResourceDesc& gDesc);

	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler{};
		friend class DeviceContext;
	};
}