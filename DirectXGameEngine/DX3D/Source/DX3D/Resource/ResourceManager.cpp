#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureResource.h>
#include <filesystem>

dx3d::ResourceManager::ResourceManager(const ResourceManagerDesc& desc) : Base(desc.base), m_context(desc.context)
{
}

dx3d::RefPtr<dx3d::Resource> dx3d::ResourceManager::createResourceFromFileConcrete(const wchar_t* file_path)
{
	std::filesystem::path resourcePath{ file_path };
	auto ext = resourcePath.extension();

	/*DX3DLogInfo("CWD: {}", std::filesystem::current_path().string().c_str());
	DX3DLogInfo("Requested resource path: {}", resourcePath.string().c_str());
	DX3DLogInfo("Absolute path: {}", std::filesystem::absolute(resourcePath).string().c_str());
	DX3DLogInfo("Exists? {}", std::filesystem::exists(std::filesystem::absolute(resourcePath)));*/

	auto it = m_resources.find(file_path);
	if (it != m_resources.end())
	{
		auto mat = std::dynamic_pointer_cast<MaterialResource>(it->second);
		if (mat)
			return std::make_shared<MaterialResource>(*mat, MaterialResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
		return it->second;
	}

	if (!std::filesystem::exists(resourcePath))
	{
		DX3DLogError("File {} doesn't exist.", resourcePath.string().c_str());
		return nullptr;
	}

	RefPtr<Resource> resPtr{};
	try
	{
		if (!ext.compare(L".hlsl") || !ext.compare(L".fx"))
			resPtr = std::make_shared<MaterialResource>(MaterialResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
		if (!ext.compare(L".jpg") || !ext.compare(L".png"))
			resPtr = std::make_shared<TextureResource>(TextureResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
	}
	catch (...)
	{
		DX3DLogError("Failed to load resource {}", resourcePath.string().c_str());
	}

	if (resPtr)
	{
		m_resources.emplace(file_path, resPtr);
		return resPtr;
	}

	return nullptr;
}

dx3d::ResourceDesc dx3d::ResourceManager::getResourceDesc(const wchar_t* file_path)
{
	return ResourceDesc{ { m_logger }, file_path, *this };
}
