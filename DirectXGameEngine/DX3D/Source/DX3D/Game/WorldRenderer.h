#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <vector>
#include <d3d11.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Mat4x4.h>

struct ImDrawData;
namespace dx3d
{
	struct alignas(16) ObjectData
	{
		Mat4x4 world{};
	};
	struct alignas(16) CameraData
	{
		Mat4x4 view{};
		Mat4x4 proj{};
		Vec4 cameraPosition{};
	};
	struct alignas(16) LightData
	{
		Vec4 lightDirection{};
		Vec4 lightColor{};
		Vec4 ambientColor{};
		Vec4 pointLightPosition{};
		Vec4 pointLightColor{};
		Vec4 pointLightSettings{};
	};

	class WorldRenderer  final: public Base
	{
	public:
		explicit WorldRenderer(const WorldRendererDesc& desc);

		void render(const World& world, SwapChain& swapChain, f32 deltaTime);
		void renderForDisplay(const World& world, Display& display, f32 deltaTime, ImDrawData* uiDrawData);	

		GraphicsDevice& getGraphicsDevice() { return m_graphicsDevice; };
		RefPtr<DeviceContext> getDeviceContext() { return m_deviceContext; };
		RefPtr<ConstantBuffer> getCameraCb() { return m_cameraCb;  };
		RefPtr<ConstantBuffer> getObjectCb() { return m_objectCb; };
		RefPtr<ConstantBuffer> getMaterialCb() { return m_materialCb; };
		RefPtr<ConstantBuffer> getLightCb() { return m_lightCb; };
		static LightData getLightData(const World& world);

	private:
		GraphicsDevice& m_graphicsDevice;
		RefPtr<DeviceContext> m_deviceContext{};
		RefPtr<ConstantBuffer> m_cameraCb{};
		RefPtr<ConstantBuffer> m_objectCb{};
		RefPtr<ConstantBuffer> m_materialCb{};
		RefPtr<ConstantBuffer> m_lightCb{};
		RefPtr<Sampler> m_sampler{};

		std::vector<Texture*> m_textures{};

		RefPtr<Rasterizer> m_rasterizer{};
		bool wireToggle{ false }; //toggle switch for wireframe mode
	};

}
