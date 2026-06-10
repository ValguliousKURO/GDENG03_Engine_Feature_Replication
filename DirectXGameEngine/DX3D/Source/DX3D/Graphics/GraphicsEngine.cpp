#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <fstream>

using namespace dx3d;

dx3d::GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& desc) : Base(desc.base)
{

	m_graphicsDevice = std::make_unique<GraphicsDevice>(GraphicsDeviceDesc{m_logger});

	auto& device = *m_graphicsDevice;
	m_deviceContext = device.createDeviceContext();

	constexpr char shaderFilePath[] = "DX3D/Assets/Shaders/Basic.hlsl";
	std::ifstream shaderStream(shaderFilePath);

	if (!shaderStream) DX3DLogThrowError("Failed to open shader file!");
	std::string shaderFileData{
		std::istreambuf_iterator<char>(shaderStream),
		std::istreambuf_iterator<char>()
	};

	auto shaderSourceCode = shaderFileData.c_str();
	auto shaderSourceCodeSize = shaderFileData.length();

	auto vs = device.compileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize,
		"VSMain", ShaderType::VertextShader});
	auto ps = device.compileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize,
		"PSMain", ShaderType::PixelShader});
	auto vsSig = device.createVertexShaderSignature({ vs });

	m_pipeline = device.createGraphicsPipelineState({ *vsSig, *ps, });

	const Vertex vertexList[] =
	{
		{
			{-0.5f, -0.5f, 0.0f},
			{1,0,0,1}
		},
		{
			{-0.5f, 0.5f, 0.0f},
			{0,1,0,1}
		},
		{
			{ 0.5f, 0.5f, 0.0f },
			{0,0,1,1}
		},

		{
			{ 0.5f, 0.5f, 0.0f },
			{0,0,1,1}
		},
		{
			{0.5f, -0.5f, 0.0f},
			{1,0,1,1}
		},
		{ 
			{-0.5f, -0.5f, 0.0f},
			{1,0,0,1}
		},
	};

	m_vb = device.createVertexBuffer({ vertexList , std::size(vertexList), sizeof(Vertex)});
	m_cb = device.createConstantBuffer({ {}, sizeof(ConstantData) });
}

dx3d::GraphicsEngine::~GraphicsEngine()
{
}

GraphicsDevice& dx3d::GraphicsEngine::getGraphicsDevice() noexcept
{
	return *m_graphicsDevice;
}

void dx3d::GraphicsEngine::render(SwapChain& swapChain, f32 deltaTime)
{
	auto& context = *m_deviceContext; 
	auto& cb = *m_cb;

	m_pos += deltaTime * 0.5f;
	m_rot += deltaTime * 3.14f;
	m_scale = std::abs(std::sin(m_rot));

	DX3DLogInfo("Pos: X:{} Y:{}", m_pos, m_pos);
	DX3DLogInfo("Rot: Z:{}", m_rot);
	DX3DLogInfo("Scale: {}", m_scale);

	auto worldMat =
		Mat4x4::scale({ m_scale,m_scale,m_scale }) *
		Mat4x4::rotateZ(m_rot) *
		Mat4x4::translate({ m_pos ,m_pos ,0 });

	ConstantData data
	{
		worldMat
	};

	context.updateConstantBuffer(cb, &data);
	context.clearAndSetBackBuffer(swapChain, 
		{ 0.66f,0.33f,0.66f,1.0f } // Background color rgba
	);
	context.SetGraphicsPipelineState(*m_pipeline);

	context.setViewportSize(swapChain.getSize());

	auto& vb = *m_vb;
	context.setVertexBuffer(vb);
	context.setConstantBuffer(cb);
	context.drawTriangleList(vb.getVertexListsize(), 0u);

	auto& device = *m_graphicsDevice;
	device.executeCommandList(context);
	swapChain.present();
}
