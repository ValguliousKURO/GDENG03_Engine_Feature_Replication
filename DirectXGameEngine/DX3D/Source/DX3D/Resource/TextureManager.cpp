#include <DX3D/Resource/TextureManager.h>

#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Resource/TextureResource.h>

#include <filesystem>

dx3d::TextureManager::TextureManager()
{
}

void dx3d::TextureManager::loadAllTextures(dx3d::ResourceManager& resManager)
{
	
    loadTexture(resManager, "DirectXGameEngine/Game/Assets/Textures/wood.jpg");
    loadTexture(resManager, "DirectXGameEngine/Game/Assets/Textures/floor.jpg");
    loadTexture(resManager, "DirectXGameEngine/Game/Assets/Textures/malinapls.jpg");
    loadTexture(resManager, "DirectXGameEngine/Game/Assets/Textures/brick.jpg");

    loadTexture(resManager, "DirectXGameEngine/Game/Assets/Textures/astonMa.jpg");
    
}

void dx3d::TextureManager::loadTexture(dx3d::ResourceManager& resManager, std::string fileName)
{
    std::filesystem::path base = std::filesystem::current_path().parent_path();
    std::string textureName = getTextureNameFromFileName(fileName);
    m_textures[textureName] = resManager.createResourceFromFile<dx3d::TextureResource>((base / fileName).c_str());
}

std::string dx3d::TextureManager::getTextureNameFromFileName(std::string fileName)
{
    // Find last slash
    size_t lastSlash = fileName.find_last_of("/\\");
    std::string fileWithExt = fileName.substr(lastSlash + 1);

    // Find last dot
    size_t lastDot = fileWithExt.find_last_of(".");
    std::string filename = fileWithExt.substr(0, lastDot);

    return filename;
}

dx3d::RefPtr<dx3d::TextureResource> dx3d::TextureManager::getTexture(std::string textureName)
{
    return m_textures[textureName];
}

std::vector<std::string> dx3d::TextureManager::getAllTextureNames()
{
    std::vector<std::string> keys;
    for (const auto& pair : m_textures) {
        keys.push_back(pair.first);
    }

    return keys;
}

std::string dx3d::TextureManager::getStringKey(dx3d::TextureResource* toFind)
{
    for (const auto& pair : m_textures) {
        if (pair.second.get() == toFind) { // shared_ptr comparison
            return pair.first;
        }
    }
    return "NOT_FOUND"; // empty string if not found
}
