#include <DX3D/Resource/Resource.h>

using namespace dx3d;

Resource::Resource(const ResourceDesc& desc) : Base(desc.base), m_path(desc.path), m_manager(desc.manager)
{
}
