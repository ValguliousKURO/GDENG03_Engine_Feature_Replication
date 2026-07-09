#include <DX3D/Graphics/RenderSystem/DeviceContext/DeviceContext.h>
#include <DX3D/Graphics/RenderSystem/SwapChain/SwapChain.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/RenderSystem/VertexBuffer/VertexBuffer.h>
#include <DX3D/Graphics/RenderSystem/IndexBuffer/IndexBuffer.h>
#include <DX3D/Graphics/RenderSystem/ConstantBuffer/ConstantBuffer.h>

dx3d::DeviceContext::DeviceContext(const GraphicsResourceDesc& gDesc): GraphicsResource(gDesc)
{
	DX3DGraphicsLogThrowOnFail(m_device.CreateDeferredContext(0, &m_context),
		"CreateDeferredContext failed!"
		);
}

void dx3d::DeviceContext::clearAndSetBackBuffer(const SwapChain& swapChain, const Vec4& color)
{
	f32 fColor[] = {color.x,color.y,color.z,color.w};
	auto rtv = swapChain.m_rtv.Get();
	auto dsv = swapChain.m_dsv.Get();

	m_context->ClearRenderTargetView(
		rtv,
		fColor
	);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);
}

void dx3d::DeviceContext::setGraphicsPipelineState(const GraphicsPipelineState& pipeline)
{
	m_context->IASetInputLayout(pipeline.m_layout.Get());
	m_context->VSSetShader(pipeline.m_vs.Get(), nullptr, 0);
	m_context->PSSetShader(pipeline.m_ps.Get(), nullptr, 0);
}

void dx3d::DeviceContext::setVertexBuffer(const VertexBuffer& buffer)
{
	auto stride = buffer.m_vertexSize;
	auto buf = buffer.m_buffer.Get();
	auto offset = 0u;
	m_context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
}

void dx3d::DeviceContext::setIndexBuffer(const IndexBuffer& buffer)
{
	m_context->IASetIndexBuffer(buffer.m_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void dx3d::DeviceContext::setViewportSize(const Rect& size)
{
	D3D11_VIEWPORT vp{};
	vp.Width = static_cast<f32>(size.width);
	vp.Height = static_cast<f32>(size.height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &vp);
}

void dx3d::DeviceContext::setConstantBuffers(const std::span<ConstantBuffer*>& buffers)
{
	if (!buffers.size())
	{
		DX3DLogError("No buffer passed to setConstantBuffers.");
		return;
	}
	if (buffers.size() > MaxConstantBuffersPerStage)
	{
		DX3DLogWarning("Number of buffers exceeds {}. Extra buffers will be ignored.", MaxConstantBuffersPerStage)
	}

	auto buffSize = static_cast<UINT>(std::min(buffers.size(), MaxConstantBuffersPerStage));
	for (auto i = 0u; i < buffSize; i++)
	{
		m_constantBuffers[i] = (buffers[i]->m_buffer.Get());
	}

	m_context->VSSetConstantBuffers(0, buffSize, m_constantBuffers);
	m_context->PSSetConstantBuffers(0, buffSize, m_constantBuffers);
}

void dx3d::DeviceContext::updateConstantBuffer(const ConstantBuffer& buffer, const std::span<const std::byte>& data)
{
	auto dataSize = static_cast<ui32>(data.size());
	if (!dataSize)
	{
		DX3DLogError("No data passed to updateConstantBuffer.");
		return;
	}
	if (dataSize > buffer.m_size)
	{
		DX3DLogWarning("Buffer size ({} bytes) exceeds the constant buffer limit ({} bytes). Extra bytes will be ignored.", dataSize, buffer.m_size);
	}

	dataSize = std::min(dataSize, buffer.m_size);

	auto buf = buffer.m_buffer.Get();
	D3D11_MAPPED_SUBRESOURCE mapped{};

	auto hr = m_context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
	{
		DX3DLogError("ID3D11DeviceContext::Map failed.");
		return;
	}

	std::memcpy(mapped.pData, data.data(), dataSize);
	m_context->Unmap(buf, 0);
}

void dx3d::DeviceContext::drawTriangleList(ui32 vertexCount, ui32 startVertexLocation)
{
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->Draw(vertexCount, startVertexLocation);
}

void dx3d::DeviceContext::drawIndexedTriangleList(ui32 indexCount, ui32 startVertexIndex, ui32 startIndexLocation)
{
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->DrawIndexed(indexCount, startIndexLocation, startVertexIndex);
}