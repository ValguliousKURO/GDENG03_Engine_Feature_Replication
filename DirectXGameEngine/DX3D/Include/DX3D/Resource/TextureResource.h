#pragma once
#include <DX3D/Resource/Resource.h>

namespace dx3d
{
	class TextureResource : public Resource
	{
	public:
		explicit TextureResource(const TextureResourceDesc& desc);
		dx3d::Texture& getTexture();
	private:
		RefPtr<Texture> m_texture{};
	};
}