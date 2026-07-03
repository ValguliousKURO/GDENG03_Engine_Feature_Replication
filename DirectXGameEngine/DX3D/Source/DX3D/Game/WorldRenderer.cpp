#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/RenderSystem/DeviceContext/DeviceContext.h>
#include <DX3D/Graphics/RenderSystem/SwapChain/SwapChain.h>
#include <DX3D/Graphics/RenderSystem/VertexBuffer/VertexBuffer.h>
#include <DX3D/Graphics/RenderSystem/IndexBuffer/IndexBuffer.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Game/GameObject.h>

#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/MeshComponent.h>

#include <DX3D/Math/Vec3.h>
#include <fstream>
#include <ranges>


dx3d::WorldRenderer::WorldRenderer(const WorldRendererDesc& desc) : Base(desc.base), m_graphicsDevice(desc.engine)
{
	auto& device = m_graphicsDevice;
	m_deviceContext = device.createDeviceContext();

	constexpr char shaderFilePath[] = "DX3D/Assets/Shaders/Basic.hlsl";
	std::ifstream shaderStream(shaderFilePath);
	if (!shaderStream) DX3DLogThrowError("Failed to open shader file.");
	std::string shaderFileData{
		std::istreambuf_iterator<char>(shaderStream),
		std::istreambuf_iterator<char>()
	};

	auto shaderSourceCode = shaderFileData.c_str();
	auto shaderSourceCodeSize = shaderFileData.length();

	auto vs = device.compileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize,
		"VSMain", ShaderType::VertexShader });
	auto ps = device.compileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize,
		"PSMain", ShaderType::PixelShader });
	auto vsSig = device.createVertexShaderSignature({ vs });

	m_pipeline = device.createGraphicsPipelineState({ *vsSig, *ps });

	/*const Vertex vertexList[] =
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
		0,1,2,
		2,3,0,

		4,5,6,
		6,7,4,

		1,6,5,
		5,2,1,

		7,0,3,
		3,4,7,

		3,2,5,
		5,4,3,

		7,6,1,
		1,0,7
	};

	m_vb = device.createVertexBuffer({ vertexList, std::size(vertexList), sizeof(Vertex) });
	m_ib = device.createIndexBuffer({ indexList, std::size(indexList) });*/

	m_cb = device.createConstantBuffer({ {}, sizeof(ConstantData) });
}

dx3d::WorldRenderer::~WorldRenderer()
{
}

void dx3d::WorldRenderer::render(const World& world, SwapChain& swapChain, f32 deltaTime)
{
	auto size = swapChain.getSize();

	auto& context = *m_deviceContext;
	context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
	context.setGraphicsPipelineState(*m_pipeline);
	context.setViewportSize(size);

	auto numComponents = 0u;
	ConstantData data{};
	{
		auto components = world.getComponents<CameraComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			data.view = component->getViewMatrix();
			component->setViewportSize(size);
			data.proj = component->getProjectionMatrix();
			break;
		}
	}

	// Render all MeshComponents
	{
		auto components = world.getComponents<MeshComponent>(numComponents);

		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			auto& transform = component->getGameObject().getTransform();
			auto mesh = component->getMesh();

			data.world = transform.getAffineWorldMatrix();

			auto& cb = *m_cb;
			context.updateConstantBuffer(cb, &data);

			auto vb = component->getOrCreateVertexBuffer(m_graphicsDevice);
			auto ib = component->getOrCreateIndexBuffer(m_graphicsDevice);

			context.setVertexBuffer(*vb);
			context.setConstantBuffer(cb);
			context.setIndexBuffer(*ib);
			context.drawIndexedTriangleList(mesh->getIndexCount(), 0u, 0u);
		}
	}

	m_graphicsDevice.executeCommandList(context);
	swapChain.present();
}