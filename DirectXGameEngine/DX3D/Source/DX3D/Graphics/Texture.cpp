#include "Texture.h"

dx3d::Texture::Texture(const TextureDesc& desc, const GraphicsResourceDesc& gDesc) : GraphicsResource(gDesc)
{
	if (!desc.size.width) DX3DLogThrowInvalidArg("Width must be non-zero.");
	if (!desc.size.height) DX3DLogThrowInvalidArg("Height must be non-zero.");
	if (!desc.pixels) DX3DLogThrowInvalidArg("Pixels must be a valid array.");

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = desc.size.width;
	texDesc.Height = desc.size.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = desc.pixels;
	initData.SysMemPitch = desc.size.width * 4;

	DX3DGraphicsLogThrowOnFail(m_device.CreateTexture2D(&texDesc, &initData, &m_texture),
		"CreateTexture2D failed.");

	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Texture2D.MostDetailedMip = 0;

	DX3DGraphicsLogThrowOnFail(m_device.CreateShaderResourceView(m_texture.Get(), &resDesc,
		&m_srv), "CreateShaderResourceView failed.");
}