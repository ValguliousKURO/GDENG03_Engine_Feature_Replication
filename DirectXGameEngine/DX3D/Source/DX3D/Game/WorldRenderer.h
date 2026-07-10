#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Mat4x4.h>
#include <vector>
namespace dx3d
{
	class WorldRenderer  final: public Base
	{
	public:
		explicit WorldRenderer(const WorldRendererDesc& desc);

		void render(const World& world, SwapChain& swapChain, f32 deltaTime);
		void renderForDisplays(const World& world, const std::vector<UniquePtr<Display>>& displays,f32 deltaTime);

	private:
		struct alignas(16) ObjectData
		{
			Mat4x4 world{};
		};
		struct alignas(16) CameraData
		{
			Mat4x4 view{};
			Mat4x4 proj{};
		};
	private:
		GraphicsDevice& m_graphicsDevice;
		RefPtr<DeviceContext> m_deviceContext{};
		RefPtr<ConstantBuffer> m_cameraCb{};
		RefPtr<ConstantBuffer> m_objectCb{};
		RefPtr<ConstantBuffer> m_materialCb{};
		RefPtr<Sampler> m_sampler{};

		std::vector<Texture*> m_textures{};
	};

}
