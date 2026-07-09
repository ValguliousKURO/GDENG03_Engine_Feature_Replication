#pragma once
#include <unordered_map>
#include <string>
#include <DX3D/Core/Common.h>
#include <DX3D/Resource/Resource.h>


namespace dx3d
{
	class ResourceManager final : public Base
	{
	public:
		explicit ResourceManager(const ResourceManagerDesc& desc);
		template<typename T>
		RefPtr<T> createResourceFromFile(const wchar_t* file_path)
		{
			return std::dynamic_pointer_cast<T>(createResourceFromFileConcrete(file_path));
		}

	private:
		RefPtr<Resource> createResourceFromFileConcrete(const wchar_t* file_path);
		ResourceDesc getResourceDesc(const wchar_t* file_path);
	private:
		std::unordered_map<std::wstring, RefPtr<Resource>> m_resources{};
		SystemContext m_context;
	};
}