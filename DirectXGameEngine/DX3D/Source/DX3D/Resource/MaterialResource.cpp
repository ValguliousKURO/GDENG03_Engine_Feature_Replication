#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Resource/ResourceManager.h>

#include <fstream>
#include <filesystem>

dx3d::MaterialResource::MaterialResource(const MaterialResourceDesc& desc) : Resource(desc.base), m_graphicsDevice(desc.graphicsDevice)
{
	std::filesystem::path shaderFile = desc.base.path;

	auto shaderFileStr = shaderFile.string();
	std::ifstream shaderStream(shaderFile);
	if (!shaderStream) DX3DLogThrowError("Failed to open shader file {}", shaderFileStr.c_str());
	std::string shaderCode{
		std::istreambuf_iterator<char>(shaderStream),
		std::istreambuf_iterator<char>()
	};

	auto vsBinary = m_graphicsDevice.compileShader({ shaderFileStr.c_str(), shaderCode.c_str(),
		shaderCode.size(), "VSMain", ShaderType::VertexShader });

	m_vsSignature = m_graphicsDevice.createVertexShaderSignature({ vsBinary });
	m_psBinary = m_graphicsDevice.compileShader({ shaderFileStr.c_str(), shaderCode.c_str(),
	shaderCode.size(), "PSMain", ShaderType::PixelShader });

	m_pipeline = m_graphicsDevice.createGraphicsPipelineState({ *m_vsSignature,*m_psBinary });
}

dx3d::MaterialResource::MaterialResource(const MaterialResource& material, const MaterialResourceDesc& desc) : Resource(desc.base), m_graphicsDevice(desc.graphicsDevice)
{
	m_vsSignature = material.m_vsSignature;
	m_psBinary = material.m_psBinary;
	m_pipeline = material.m_pipeline;
}

const dx3d::GraphicsPipelineState& dx3d::MaterialResource::getGraphicsPipelineState() const noexcept
{
	return *m_pipeline;
}

void dx3d::MaterialResource::setData(const std::span<const std::byte>& data)
{
	if (!data.size())
	{
		DX3DLogError("No material data provided.")
			return;
	}
	if (data.size() > MaxDataSize)
	{
		DX3DLogWarning("Material data size ({} bytes) exceeds the maximum allowed size of {} bytes. Data will be truncated.", data.size(), MaxDataSize)
	}

	auto size = std::min(data.size(), MaxDataSize);
	memcpy(m_data, data.data(), size);
	m_dataSize = size;
}

const std::span<const std::byte> dx3d::MaterialResource::getData() const noexcept
{
	return m_data;
}