#include "Rasterizer.h"

dx3d::Rasterizer::Rasterizer(const RasterizerDesc& desc, const GraphicsResourceDesc& gDesc)
	:GraphicsResource(gDesc)
{
	// set raster - note - desc means descriptor all the possible settings and values to be set before being used
	D3D11_RASTERIZER_DESC rDesc{};
	rDesc.FillMode = desc.isWire ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rDesc.CullMode = D3D11_CULL_BACK; // cull back facing vertses front face will disable em
	rDesc.FrontCounterClockwise = false; // front face is clockwise by default 
	//DEPTH BIASES 
	rDesc.DepthBias = 0;
	rDesc.SlopeScaledDepthBias = 0;
	rDesc.DepthBiasClamp = 0;
	rDesc.DepthClipEnable = true; // enable depth clipping
	rDesc.ScissorEnable = false; // dont have necessary rect files for scissor test
	// sampling
	rDesc.MultisampleEnable = false; 
	rDesc.AntialiasedLineEnable = false;

	DX3DGraphicsLogThrowOnFail(
		m_device.CreateRasterizerState(&rDesc, &m_rasterizer), 
		"Raster Error.");


}

Microsoft::WRL::ComPtr<ID3D11RasterizerState> dx3d::Rasterizer::getRasterizer()
{
	return m_rasterizer;
}