#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <string>

namespace dx3d
{
	class Resource : public Base
	{
	public:
		explicit Resource(const ResourceDesc& desc);
	protected:
		std::wstring m_path{};
		ResourceManager& m_manager;
	};

}