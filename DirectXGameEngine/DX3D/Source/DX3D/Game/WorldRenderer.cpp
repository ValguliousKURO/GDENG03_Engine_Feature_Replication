#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Game/GameObject.h>

#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>

#include <fstream>
#include <ranges>

dx3d::WorldRenderer::WorldRenderer(const WorldRendererDesc& desc) : Base(desc.base), m_graphicsDevice(desc.engine)
{
	auto& device = m_graphicsDevice;
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
		{{-0.5f,-0.5f,-0.5f}, {1,0,0,1}},
		{{-0.5f,0.5f,-0.5f}, {0,1,0,1} },
		{{0.5f,0.5f,-0.5f},  {0,0,1,1}},
		{{0.5f,-0.5f,-0.5f}, {1,0,1,1}},

		{{0.5f,-0.5f,0.5f}, {1,0,1,1}},
		{{0.5f,0.5f,0.5f}, {0,0,1,1}},
		{{-0.5f,0.5f,0.5f}, {0,1,0,1}},
		{{-0.5f,-0.5f,0.5f}, {1,0,0,1}}
	};

	const ui32 indexList[] =
	{
		//winding order: clockwise
		// 
		//front face indices
		0,1,2,  //first triangle
		2,3,0,  //second triangle
		//back face indices
		4,5,6,
		6,7,4,
		//top face indices
		1,6,5,
		5,2,1,
		//bottom face indices
		7,0,3,
		3,4,7,
		//right face indices
		3,2,5,
		5,4,3,
		//left face indices
		7,6,1,
		1,0,7
	};

	m_vb = device.createVertexBuffer({ vertexList , std::size(vertexList), sizeof(Vertex)});
	m_cb = device.createConstantBuffer({ {}, sizeof(ConstantData) });
	m_ib = device.createIndexBuffer({ indexList, std::size(indexList) });
}

dx3d::WorldRenderer::~WorldRenderer()
{
}

void dx3d::WorldRenderer::render(const World& world, SwapChain& swapChain, f32 deltaTime)
{

	//orthographic camera setup
	auto size = swapChain.getSize();
	auto aspect = static_cast<f32>(size.width) / size.height;
	auto unitsPerScreenHeight = 5.0f;
	auto viewHeight = unitsPerScreenHeight;
	auto viewWidth = unitsPerScreenHeight * aspect;

	auto& context = *m_deviceContext;
	context.clearAndSetBackBuffer(swapChain, 
		{ 0.66f,0.33f,0.66f,1.0f } // Background color rgba
	);
	context.SetGraphicsPipelineState(*m_pipeline);
	context.setViewportSize(size);

	auto numComponents = 0u;
	auto components = world.getComponents<CubeComponent>(numComponents);
	ConstantData data{};

	for (auto i : std::views::iota(0u, numComponents))
	{
		auto component = components[i];
		auto& transform = component->getGameObject().getTransform();

		data =
			ConstantData
		{
			transform.getWorldMatrix(),
			Mat4x4::orthoLH(viewWidth, viewHeight, -10.0f, 10.0f)
		};

		auto& cb = *m_cb;
		context.updateConstantBuffer(cb, &data);

		auto& vb = *m_vb;
		auto& ib = *m_ib;
		context.setVertexBuffer(vb);
		context.setConstantBuffer(cb);
		context.setIndexBuffer(ib);
		context.drawIndexedTriangleList(ib.getIndexListSize(), 0u, 0u);
	}

	m_graphicsDevice.executeCommandList(context);
	swapChain.present();
}
