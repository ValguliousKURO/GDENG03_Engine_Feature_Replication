#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Game/Display.h>

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

#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureResource.h>

#include <DX3D/Math/Vec3.h>
#include <fstream>
#include <ranges>

// ADDED: DirectX 11 backend renders ImGui after the engine command list has been executed.
#include <imgui_impl_dx11.h>

// event broadcast needed
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <iostream>

dx3d::WorldRenderer::WorldRenderer(const WorldRendererDesc& desc) : Base(desc.base), m_graphicsDevice(desc.engine)
{
	auto& device = m_graphicsDevice;
	m_deviceContext = device.createDeviceContext();

	//m_cb = device.createConstantBuffer({ {}, sizeof(ConstantData) });

	m_textures.reserve(32);

	m_objectCb = device.createConstantBuffer({ {}, sizeof(ObjectData) });
	m_cameraCb = device.createConstantBuffer({ {}, sizeof(CameraData) });
	m_materialCb = device.createConstantBuffer({ {}, dx3d::MaterialResource::MaxDataSize });
	m_lightCb = device.createConstantBuffer({ {}, sizeof(LightData) });

	m_sampler = device.createSampler({});



	// create one wireframe rasterizer; this might not work but am gonna test
	dx3d::RasterizerDesc rd{};
	rd.isWire = true;
	m_rasterizer = m_graphicsDevice.createRasterizer(rd);

	// register for wireframe toggle events
	dx3d::EventBroadcastManager::getInstance().addObserver(dx3d::EventNames::WIREFRAME_TOGGLE, [this]()
		{
		if(wireToggle)
		{
			wireToggle = false;
		}
		else
		{
			wireToggle = true;
		}
		std::cout << "Wireframe mode toggled: " << (wireToggle ? "ON" : "OFF") << std::endl;
		
		});
}

void dx3d::WorldRenderer::render(const World& world, SwapChain& swapChain, f32 deltaTime)
{
	auto size = swapChain.getSize();

	auto& context = *m_deviceContext;
	context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
	context.setViewportSize(size);

	Sampler* samplers[] = { m_sampler.get() };
	context.setSamplers(std::span<Sampler*>{samplers});

	auto numComponents = 0u;

	auto& cameraCb = *m_cameraCb;
	auto& objectCb = *m_objectCb;
	auto& materialCb = *m_materialCb;

	{
		CameraData cameraData{};
		auto components = world.getComponents<CameraComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{

			auto component = components[i];
			cameraData.view = component->getViewMatrix();
			component->setViewportSize(size);
			cameraData.proj = component->getProjectionMatrix();
			cameraData.cameraPosition = Vec4(
				component->getGameObject().getTransform().getPosition().x,
				component->getGameObject().getTransform().getPosition().y,
				component->getGameObject().getTransform().getPosition().z,
				1.0f
			);
			context.updateConstantBuffer(cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));

			if (wireToggle && m_rasterizer) context.setRasterizerState(*m_rasterizer);
			else context.clearRaster();
			break;
		}
	}

	{
		LightData lightData{};
		lightData.lightDirection = Vec4(0.577f, -0.577f, 0.577f, 0.0f);
		lightData.lightColor = Vec4(1.0f, 0.95f, 0.9f, 1.0f);
		lightData.ambientColor = Vec4(0.2f, 0.22f, 0.25f, 1.0f);
		context.updateConstantBuffer(*m_lightCb, std::as_bytes(std::span{ &lightData, 1 }));
	}

	// Render all MeshComponents
	{
		ObjectData objectData{};
		auto components = world.getComponents<MeshComponent>(numComponents);

		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			auto& transform = component->getGameObject().getTransform();
			auto mesh = component->getMesh();

			auto material = component->getMaterial();

			if (material)
			{
				objectData.world = transform.getAffineWorldMatrix();

				context.setGraphicsPipelineState(material->getGraphicsPipelineState());
				context.updateConstantBuffer(objectCb, std::as_bytes(std::span{ &objectData, 1 }));
				context.updateConstantBuffer(materialCb, material->getData());
				ConstantBuffer* cbs[] = { &objectCb, &cameraCb, &materialCb, m_lightCb.get() };
				context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

				auto vb = component->getOrCreateVertexBuffer(m_graphicsDevice);
				auto ib = component->getOrCreateIndexBuffer(m_graphicsDevice);

				/*context.setVertexBuffer(*vb);
				context.setIndexBuffer(*ib);
				context.drawIndexedTriangleList(mesh->getIndexCount(), 0u, 0u);*/

				m_textures.clear();
				m_textures.resize(material->getNumTextures());
				for (auto t : std::views::iota(0u, m_textures.size()))
				{
					auto tex = material->getTexture(t);
					if (tex) m_textures[t] = &tex->getTexture();
				}
				context.setTextures(std::span<Texture*>{m_textures});

				context.setVertexBuffer(*vb);
				context.setIndexBuffer(*ib);
				context.drawIndexedTriangleList(mesh->getIndexCount(), 0u, 0u);
			}
		}

		m_graphicsDevice.executeCommandList(context);
		swapChain.present();
	}
}

void dx3d::WorldRenderer::renderForDisplays(const World& world, const std::vector<UniquePtr<Display>>& displays, f32 deltaTime, ImDrawData* uiDrawData)
{
	for (auto& display : displays) // get all displats here
	{
		HWND hwnd = static_cast<HWND>(display->getHandle());
		if (IsIconic(hwnd)) // skip minimized windows
			continue;
		// get the swap chain for this display and its size a
		auto& swapChain = display->getSwapChain();
		auto size = swapChain.getSize();

		// get dev context
		auto& context = *m_deviceContext;
		context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
		context.setViewportSize(size);


		Sampler* samplers[] = { m_sampler.get() };
		context.setSamplers(std::span<Sampler*>{samplers});

		// Use and set display camera
		if (auto* camera = display->getCamera())
		{
			CameraData cameraData{};
			cameraData.view = camera->getViewMatrix();
			camera->setViewportSize(size);
			cameraData.proj = camera->getProjectionMatrix();
			cameraData.cameraPosition = Vec4(
				camera->getGameObject().getTransform().getPosition().x,
				camera->getGameObject().getTransform().getPosition().y,
				camera->getGameObject().getTransform().getPosition().z,
				1.0f
			);
			context.updateConstantBuffer(*m_cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));
		}

		// IMPORTANT: renderForDisplays is the active display path, so bind the
		// selected rasterizer here before submitting any mesh draw calls.
		if (display->getRenderMode() == Display::RenderMode::Wireframe && m_rasterizer) context.setRasterizerState(*m_rasterizer);
		else context.clearRaster();

		{
			LightData lightData{};
			lightData.lightDirection = Vec4(0.577f, -0.577f, 0.577f, 0.0f);
			lightData.lightColor = Vec4(1.0f, 0.95f, 0.9f, 1.0f);
			lightData.ambientColor = Vec4(0.2f, 0.22f, 0.25f, 1.0f);
			context.updateConstantBuffer(*m_lightCb, std::as_bytes(std::span{ &lightData, 1 }));
		}

		// Render meshes (same as your existing render code)
		ui32 numComponents = 0;
		auto components = world.getComponents<MeshComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			auto& transform = component->getGameObject().getTransform();
			auto mesh = component->getMesh();
			auto material = component->getMaterial();

			if (material)
			{
				ObjectData objectData{};
				objectData.world = transform.getAffineWorldMatrix();

				context.setGraphicsPipelineState(material->getGraphicsPipelineState());
				context.updateConstantBuffer(*m_objectCb, std::as_bytes(std::span{ &objectData, 1 }));
				context.updateConstantBuffer(*m_materialCb, material->getData());
				ConstantBuffer* cbs[] = { m_objectCb.get(), m_cameraCb.get(), m_materialCb.get(), m_lightCb.get() };
				context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

				auto vb = component->getOrCreateVertexBuffer(m_graphicsDevice);
				auto ib = component->getOrCreateIndexBuffer(m_graphicsDevice);

				m_textures.clear();
				m_textures.resize(material->getNumTextures());
				for (auto t : std::views::iota(0u, m_textures.size()))
				{
					auto tex = material->getTexture(t);
					if (tex) m_textures[t] = &tex->getTexture();
				}
				context.setTextures(std::span<Texture*>{m_textures});

				context.setVertexBuffer(*vb);
				context.setIndexBuffer(*ib);
				context.drawIndexedTriangleList(mesh->getIndexCount(), 0u, 0u);
			}
		}

		m_graphicsDevice.executeCommandList(context);
		if (uiDrawData)
		{
			// Scene rendering uses a deferred context; ImGui uses the immediate
			// context. Bind *this* display's target before every ImGui submission.
			// Replaying the same draw data here makes the UI visible in every engine
			// display instead of only the display that created the ImGui frame.
			auto* renderTarget = swapChain.getRenderTargetView();
			m_graphicsDevice.getNativeContext()->OMSetRenderTargets(1, &renderTarget, nullptr);

			ImGui_ImplDX11_RenderDrawData(uiDrawData);
		}
		swapChain.present();
	}
}

void dx3d::WorldRenderer::renderForDisplay(const World& world, Display& display, f32 deltaTime, ImDrawData* uiDrawData)
{
	HWND hwnd = static_cast<HWND>(display.getHandle());
	if (IsIconic(hwnd))
		return;

	auto& swapChain = display.getSwapChain();
	auto size = swapChain.getSize();

	auto& context = *m_deviceContext;
	context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
	context.setViewportSize(size);

	Sampler* samplers[] = { m_sampler.get() };
	context.setSamplers(std::span<Sampler*>{samplers});

	if (auto* camera = display.getCamera())
	{
		CameraData cameraData{};
		cameraData.view = camera->getViewMatrix();
		camera->setViewportSize(size);
		cameraData.proj = camera->getProjectionMatrix();
		cameraData.cameraPosition = Vec4(
			camera->getGameObject().getTransform().getPosition().x,
			camera->getGameObject().getTransform().getPosition().y,
			camera->getGameObject().getTransform().getPosition().z,
			1.0f
		);
		context.updateConstantBuffer(*m_cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));
	}

	if (display.getRenderMode() == Display::RenderMode::Wireframe && m_rasterizer) context.setRasterizerState(*m_rasterizer);
	else context.clearRaster();

	{
		LightData lightData{};
		lightData.lightDirection = Vec4(0.577f, -0.577f, 0.577f, 0.0f);
		lightData.lightColor = Vec4(1.0f, 0.95f, 0.9f, 1.0f);
		lightData.ambientColor = Vec4(0.2f, 0.22f, 0.25f, 1.0f);
		context.updateConstantBuffer(*m_lightCb, std::as_bytes(std::span{ &lightData, 1 }));
	}

	ui32 numComponents = 0;
	auto components = world.getComponents<MeshComponent>(numComponents);
	for (auto i : std::views::iota(0u, numComponents))
	{
		auto component = components[i];
		auto& transform = component->getGameObject().getTransform();
		auto mesh = component->getMesh();
		auto material = component->getMaterial();

		if (material)
		{
			ObjectData objectData{};
			objectData.world = transform.getAffineWorldMatrix();

			context.setGraphicsPipelineState(material->getGraphicsPipelineState());
			context.updateConstantBuffer(*m_objectCb, std::as_bytes(std::span{ &objectData, 1 }));
			context.updateConstantBuffer(*m_materialCb, material->getData());
			ConstantBuffer* cbs[] = { m_objectCb.get(), m_cameraCb.get(), m_materialCb.get(), m_lightCb.get() };
			context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

			auto vb = component->getOrCreateVertexBuffer(m_graphicsDevice);
			auto ib = component->getOrCreateIndexBuffer(m_graphicsDevice);

			m_textures.clear();
			m_textures.resize(material->getNumTextures());
			for (auto t : std::views::iota(0u, m_textures.size()))
			{
				auto tex = material->getTexture(t);
				if (tex) m_textures[t] = &tex->getTexture();
			}
			context.setTextures(std::span<Texture*>{m_textures});

			context.setVertexBuffer(*vb);
			context.setIndexBuffer(*ib);
			context.drawIndexedTriangleList(mesh->getIndexCount(), 0u, 0u);
		}
	}

	m_graphicsDevice.executeCommandList(context);
	if (uiDrawData)
	{
		auto* renderTarget = swapChain.getRenderTargetView();
		m_graphicsDevice.getNativeContext()->OMSetRenderTargets(1, &renderTarget, nullptr);
		ImGui_ImplDX11_RenderDrawData(uiDrawData);
	}
	swapChain.present();
}
