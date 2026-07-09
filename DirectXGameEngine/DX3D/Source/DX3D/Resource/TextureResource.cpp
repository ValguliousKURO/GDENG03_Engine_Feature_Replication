#include <DX3D/Resource/TextureResource.h>
#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <fstream>
#include <filesystem>
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include <stb-image/stb_image.h>

dx3d::TextureResource::TextureResource(const TextureResourceDesc& desc) : Resource(desc.base)
{
    std::filesystem::path textureFile = desc.base.path;
    auto textureFileStr = textureFile.string();

    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = nullptr;

    // Use _wfopen_s for wide-char paths
    FILE* f = nullptr;
    errno_t err = _wfopen_s(&f, desc.base.path, L"rb");
    if (err != 0 || !f)
    {
        DX3DLogThrowError("Failed to open texture file {} (errno {})", textureFileStr.c_str(), err);
    }

    // Load image data from the FILE*
    pixels = stbi_load_from_file(f, &width, &height, &channels, STBI_rgb_alpha);
    fclose(f);

    if (!pixels)
    {
        DX3DLogThrowError("Failed to load texture file {} (stb error: {})", textureFileStr.c_str(), stbi_failure_reason());
    }

    try
    {
        m_texture = desc.graphicsDevice.createTexture({ { width, height }, pixels });
    }
    catch (...)
    {
        stbi_image_free(pixels);
        throw;
    }

    stbi_image_free(pixels);
}

dx3d::Texture& dx3d::TextureResource::getTexture()
{
    return *m_texture;
}
