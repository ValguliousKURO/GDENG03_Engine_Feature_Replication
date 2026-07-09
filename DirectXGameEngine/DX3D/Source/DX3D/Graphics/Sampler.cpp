#include "Sampler.h"

dx3d::Sampler::Sampler(const SamplerDesc& desc, const GraphicsResourceDesc& gDesc) : GraphicsResource(gDesc)
{
	D3D11_SAMPLER_DESC sampler_desc = {};
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	DX3DGraphicsLogThrowOnFail(m_device.CreateSamplerState(&sampler_desc, &m_sampler), "CreateSamplerState failed.");
}