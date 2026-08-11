#include <DX3D/UI/SceneViewportUI.h>

#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/RenderSystem/DeviceContext/DeviceContext.h>

#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/TransformComponent.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Game/GameObject.h>

#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureResource.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

#include <ranges>

dx3d::SceneViewportUI::SceneViewportUI(const BaseDesc& desc, World& world, WorldRenderer& worldRenderer)
	: BaseUI(desc), m_world(world), m_graphicsDevice(worldRenderer.getGraphicsDevice()), m_deviceContext(worldRenderer.getDeviceContext())
{
	m_cameraCb = worldRenderer.getCameraCb();
	m_objectCb = worldRenderer.getObjectCb();
	m_materialCb = worldRenderer.getMaterialCb();
	m_lightCb = worldRenderer.getLightCb();
}

dx3d::SceneViewportUI::~SceneViewportUI()
{
}

void dx3d::SceneViewportUI::draw()
{
	renderWorldViewport();
}

void dx3d::SceneViewportUI::renderToTexture(int width, int height)
{
	auto* ctx = m_deviceContext->getNativeContext();
	ctx->OMSetRenderTargets(1, m_offscreenRTV.GetAddressOf(), m_offscreenDSV.Get());

	// Clear both color and depth
	const float clearColor[4] = { 0.27f, 0.39f, 0.55f, 1.0f };
	ctx->ClearRenderTargetView(m_offscreenRTV.Get(), clearColor);
	ctx->ClearDepthStencilView(m_offscreenDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	ctx->RSSetViewports(1, &vp);

	// Call the shared scene rendering
	renderScene(width, height);
}

void dx3d::SceneViewportUI::createOffscreenTarget(int width, int height)
{
	auto* device = m_graphicsDevice.getNativeDevice();
	if (!device) {
		DX3DLogThrowError("Native Device Not Initialized!");
		return;
	}

	// --- Color texture ---
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_offscreenTex);
	if (FAILED(hr)) { DX3DLogError("CreateTexture2D failed!"); return; }

	hr = device->CreateRenderTargetView(m_offscreenTex.Get(), nullptr, &m_offscreenRTV);
	if (FAILED(hr)) { DX3DLogError("CreateRenderTargetView failed!"); return; }

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(m_offscreenTex.Get(), &srvDesc, &m_offscreenSRV);
	if (FAILED(hr)) { DX3DLogError("CreateShaderResourceView failed!"); return; }

	// --- Depth/stencil texture ---
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTex;
	hr = device->CreateTexture2D(&depthDesc, nullptr, &depthTex);
	if (FAILED(hr)) { DX3DLogError("CreateDepthTexture failed!\n"); return; }

	hr = device->CreateDepthStencilView(depthTex.Get(), nullptr, &m_offscreenDSV);
	if (FAILED(hr)) { DX3DLogError("CreateDepthStencilView failed!\n"); return; }
}

void dx3d::SceneViewportUI::renderWorldViewport()
{
	ImGui::Begin(m_name.c_str());
	ImVec2 avail = ImGui::GetContentRegionAvail();

	if (ImGui::IsWindowFocused()) {
		Parameters param;
		param.PutExtra("CameraID", m_camera_ID);
		EventBroadcastManager::getInstance().postEvent(
			EventNames::ON_VIEWPORT_FOCUSED,
			param
		);
	}

	// Recreate offscreen target if size changed and valid
	static int texW = 0, texH = 0;
	int newW = (int)avail.x;
	int newH = (int)avail.y;

	if (newW > 0 && newH > 0 && (newW != texW || newH != texH)) {
		texW = newW;
		texH = newH;
		createOffscreenTarget(texW, texH);
	}

	// Only render if we have a valid target
	if (texW > 0 && texH > 0 && m_offscreenSRV) {
		renderToTexture(texW, texH);
		ImGui::Image((ImTextureID)m_offscreenSRV.Get(), avail);
	}

	ImGui::End();
}

void dx3d::SceneViewportUI::renderScene(int width, int height)
{
	// Camera setup (similar to renderForDisplay)

	auto& context = *m_deviceContext;
	ui32 numComponents = 0;

	//Getting the camera
	GameObject* cam_obj = m_world.getGameObjectById(m_camera_ID);
	CameraComponent* cam_component = cam_obj->createOrGetComponent<CameraComponent>();

	/*auto cameras = m_world.getComponents<CameraComponent>(numComponents);
	CameraComponent* cam_component = nullptr;*/

	/*for (int i = 0; i < numComponents; i++)
	{
		if (cameras[i]->getID() == m_camera_ID)
		{
			cam = cameras[i];
		}
	}*/

	if (cam_component != nullptr) {
		CameraData cameraData{};
		cameraData.view = cam_component->getViewMatrix();
		cam_component->setViewportSize({ width, height });
		cameraData.proj = cam_component->getProjectionMatrix();
		cameraData.cameraPosition = Vec4(
			cam_component->getGameObject().getTransform().getPosition().x,
			cam_component->getGameObject().getTransform().getPosition().y,
			cam_component->getGameObject().getTransform().getPosition().z,
			1.0f
		);
		context.updateConstantBuffer(*m_cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));
	}

	// Light setup
	auto lightData = WorldRenderer::getLightData(m_world);
	context.updateConstantBuffer(*m_lightCb, std::as_bytes(std::span{ &lightData, 1 }));

	// Mesh loop (copied from renderForDisplay)
	auto components = m_world.getComponents<MeshComponent>(numComponents);
	for (auto i : std::views::iota(0u, numComponents)) {
		auto component = components[i];
		if (!component->getGameObject().isEnabled() || component->getGameObject().isDeleted())
			continue;

		auto& transform = component->getGameObject().getTransform();
		auto mesh = component->getMesh();
		auto material = component->getMaterial();

		if (material) {
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
			for (auto t : std::views::iota(0u, m_textures.size())) {
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
}
